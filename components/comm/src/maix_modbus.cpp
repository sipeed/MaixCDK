#include "maix_modbus.hpp"
#include "maix_log.hpp"

#include <cstring>          // std::memcpy
#include <stdexcept>        // std::runtime_error
#include <unistd.h>         // close
#include <limits>           // std::numeric_limits
#include <sys/select.h>     // select
#include <sstream>          // std::stringstream

namespace maix::comm::modbus {

using namespace ::maix::log;

static inline void __error_and_throw__(const std::string& msg) noexcept(false)
{
    log::error(msg.c_str());
    throw std::runtime_error(msg);
}

static inline void __throw_in_maixpy__(const std::string& msg) noexcept(false)
{
    log::error(msg.c_str());
#if CONFIG_BUILD_WITH_MAIXPY
    throw std::runtime_error(msg);
#endif
}

/****************************** Slave *********************************/

const std::string Slave::TAG() const noexcept
{
    return "[Maix Modbus Slave]";
}

void Slave::header_init()
{
    this->header_len_ = ::modbus_get_header_length(this->ctx_.get());
    if (this->debug_) {
        log::info("%s Get header index: %d", this->TAG().c_str(), this->header_len_);
    }
}

void Slave::debug_init()
{
    if (::modbus_set_debug(this->ctx_.get(), this->debug_) < 0) {
        const std::string msg(this->TAG()+" modbus set debug failed!"+std::string(::modbus_strerror(errno)));
        __error_and_throw__(msg);
    }
    if (this->debug_) {
        log::info("%s set debug succ", this->TAG().c_str());
    }
}

void Slave::mapping_init()
{
    this->mb_mapping_.reset(
        ::modbus_mapping_new_start_address(
            this->registers_info_.coils.start_address,              this->registers_info_.coils.size,
            this->registers_info_.discrete_inputs.start_address,    this->registers_info_.discrete_inputs.size,
            this->registers_info_.holding_registers.start_address,  this->registers_info_.holding_registers.size,
            this->registers_info_.input_registers.start_address,    this->registers_info_.input_registers.size
        )
    );

    if (this->mb_mapping_.get() == nullptr) {
        const std::string msg(this->TAG()+" Failed to allocate the mapping!"+std::string(::modbus_strerror(errno)));
        __error_and_throw__(msg);
    }

    std::memset(this->mb_mapping_.get()->tab_bits,              0x00, this->registers_info_.coils.size);
    std::memset(this->mb_mapping_.get()->tab_input_bits,        0x00, this->registers_info_.discrete_inputs.size);
    std::memset(this->mb_mapping_.get()->tab_registers,         0x00, this->registers_info_.holding_registers.size);
    std::memset(this->mb_mapping_.get()->tab_input_registers,   0x00, this->registers_info_.input_registers.size);

    if (this->debug_) {
        log::info("%s mapping init succ", this->TAG().c_str());
    }
}

Slave::Slave(maix::comm::modbus::Mode mode, const std::string& ip_or_device,
            uint32_t coils_start, uint32_t coils_size,
            uint32_t discrete_start, uint32_t discrete_size,
            uint32_t holding_start, uint32_t holding_size,
            uint32_t input_start, uint32_t input_size,
            int rtu_baud, uint8_t rtu_slave,
            int tcp_port, bool debug)
{
    this->debug_ = debug;
    this->registers_info_.coils.start_address = coils_start;
    this->registers_info_.coils.size = coils_size;
    this->registers_info_.discrete_inputs.start_address = discrete_start;
    this->registers_info_.discrete_inputs.size = discrete_size;
    this->registers_info_.holding_registers.start_address = holding_start;
    this->registers_info_.holding_registers.size = holding_size;
    this->registers_info_.input_registers.start_address = input_start;
    this->registers_info_.input_registers.size = input_size;

    if (mode == Mode::RTU)
        this->rtu_init(ip_or_device, rtu_baud, rtu_slave);
    else if (mode == Mode::TCP)
        this->tcp_init(ip_or_device, tcp_port);
    else
        __error_and_throw__(this->TAG()+" Unknown Mode!");

    if (this->set_timeout(0, 0) != ::maix::err::Err::ERR_NONE) {
        const std::string errmsg(this->TAG()+" Set timeout failed");
        log::error(errmsg.c_str());
        throw std::runtime_error(errmsg);
    }
}

Slave::Slave(maix::comm::modbus::Mode mode, const std::string& ip_or_device,
            const Registers& registers,
            int rtu_baud, uint8_t rtu_slave,
            int tcp_port, bool debug)
{
    this->debug_ = debug;
    std::memcpy(&this->registers_info_, &registers, sizeof(Registers));

    if (mode == Mode::RTU)
        this->rtu_init(ip_or_device, rtu_baud, rtu_slave);
    else if (mode == Mode::TCP)
        this->tcp_init(ip_or_device, tcp_port);
    else
        __error_and_throw__(this->TAG()+" Unknown Mode!");

    if (this->set_timeout(0, 0) != ::maix::err::Err::ERR_NONE) {
        const std::string errmsg(this->TAG()+" Set timeout failed");
        log::error(errmsg.c_str());
        throw std::runtime_error(errmsg);
    }
}

::maix::err::Err Slave::set_timeout(uint32_t sec, uint32_t usec)
{
    // nonblocking
    if (sec == 0 && usec == 0) {
        if (this->curr_timeout_sec_ == 0 && this->curr_timeout_usec_ == 1)
            return ::maix::err::Err::ERR_NONE;
        if (this->debug_) {
            log::info("%s Timeout: 0", this->TAG().c_str());
        }
        this->curr_timeout_sec_ = 0;
        this->curr_timeout_usec_ = 1;
        if (::modbus_set_indication_timeout(this->ctx_.get(), this->curr_timeout_sec_, this->curr_timeout_usec_) < 0) {
            std::string msg(this->TAG()+" set timeout failed");
            log::warn(msg.c_str());
            return ::maix::err::Err::ERR_RUNTIME;
            // __error_and_throw__(msg);
        }
        if (::modbus_set_response_timeout(this->ctx_.get(), this->curr_timeout_sec_, this->curr_timeout_usec_) < 0) {
            std::string msg(this->TAG()+" set timeout failed");
            log::warn(msg.c_str());
            return ::maix::err::Err::ERR_RUNTIME;
        }
        return ::maix::err::Err::ERR_NONE;
    }

    // blocking
    if (usec > 999999 || sec == std::numeric_limits<uint32_t>::max()) {
        if (this->curr_timeout_sec_ == 0 && this->curr_timeout_usec_ == 0)
            return ::maix::err::Err::ERR_NONE;
        if (this->debug_) {
            log::info("%s Timeout: max", this->TAG().c_str());
        }
        this->curr_timeout_sec_ = 0;
        this->curr_timeout_usec_ = 0;
        if (::modbus_set_indication_timeout(this->ctx_.get(), this->curr_timeout_sec_, this->curr_timeout_usec_) < 0) {
            std::string msg(this->TAG()+" set timeout failed");
            log::warn(msg.c_str());
            return ::maix::err::Err::ERR_RUNTIME;
            // __error_and_throw__(msg);
        }
        return ::maix::err::Err::ERR_NONE;
    }

    if (this->curr_timeout_sec_ == sec && this->curr_timeout_usec_ == usec) {
        return ::maix::err::Err::ERR_NONE;
    }
    if (this->debug_) {
        log::info("%s Timeout: %u sec %u usec",
            this->TAG().c_str(), this->curr_timeout_sec_, this->curr_timeout_usec_);
    }
    this->curr_timeout_sec_ = sec;
    this->curr_timeout_usec_ = usec;
    if (::modbus_set_indication_timeout(this->ctx_.get(), this->curr_timeout_sec_, this->curr_timeout_usec_) < 0) {
        std::string msg(this->TAG()+" set timeout failed");
        log::warn(msg.c_str());
        return ::maix::err::Err::ERR_RUNTIME;
        // __error_and_throw__(msg);
    }
    return ::maix::err::Err::ERR_NONE;
}

void Slave::rtu_init(const std::string& device, int baud, uint8_t slave)
{
    if (this->debug_) {
        log::info("%s Mode: RTU, Port: %s, Baudrate: %d-8N1, Slave addr: %u.",
            this->TAG().c_str(), device.c_str(), baud, slave);
    }

    this->ctx_.reset(::modbus_new_rtu(device.c_str(), baud, 'N', 8, 1));
    if (this->ctx_.get() == nullptr) {
        const std::string msg(this->TAG()+" malloc failed!");
        __error_and_throw__(msg);
    }

    if (this->debug_) {
        log::info("%s alloc ctx succ", this->TAG().c_str());
    }

    if (::modbus_set_slave(this->ctx_.get(), slave) < 0) {
        const std::string msg(this->TAG()+" modbus set slave failed!");
        __error_and_throw__(msg);
    }

    if (this->debug_) {
        log::info("%s set slave succ", this->TAG().c_str());
    }

    this->header_init();

    this->debug_init();

    this->mapping_init();

    int rc = ::modbus_connect(this->ctx_.get());
    if (rc < 0) {
        const std::string msg(this->TAG()+" Connect failed!"+std::string(::modbus_strerror(errno)));
        __error_and_throw__(msg);
    }
}

void Slave::tcp_init(const std::string& ip, int port)
{
    if (this->debug_) {
        log::info("%s Mode: TCP, Port: %d",
            this->TAG().c_str(), port);
    }

    this->ctx_.reset(::modbus_new_tcp(nullptr, port));
    if (this->ctx_.get() == nullptr) {
        const std::string msg(this->TAG()+" malloc failed!");
        __error_and_throw__(msg);
    }

    this->header_init();

    this->debug_init();

    this->mapping_init();

    this->socket_tcp_ = ::modbus_tcp_listen(this->ctx_.get(), 1);

    this->tcp_listener_ = std::make_unique<std::thread>([this](){
        while (!this->tcp_listener_need_exit_) {
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(this->socket_tcp_, &read_fds);

            struct timeval timeout;
            timeout.tv_sec = 1; // timeout 1 sec
            timeout.tv_usec = 0;

            int select_result = ::select(this->socket_tcp_ + 1, &read_fds, nullptr, nullptr, &timeout);
            if (select_result > 0) {
                int new_socket = ::modbus_tcp_accept(this->ctx_.get(), &socket_tcp_);
                if (new_socket < 0 && this->debug_) {
                    const std::string msg(this->TAG() + " tcp accept failed! " + std::string(::modbus_strerror(errno)));
                    log::warn(msg.c_str());
                } else if (this->debug_) {
                    const std::string msg(this->TAG() + " new tcp connected!");
                    log::info(msg.c_str());
                }
            } else if (select_result < 0 && this->debug_) {
                const std::string msg(this->TAG() + " select failed! " + std::string(::modbus_strerror(errno)));
                log::warn(msg.c_str());
            }
        }
    });
}

Slave::~Slave()
{
    if (this->tcp_listener_ != nullptr) {
        this->tcp_listener_need_exit_ = true;
        log::info("%s waiting for tcp listener exit...", this->TAG().c_str());
        this->tcp_listener_->join();
        this->tcp_listener_need_exit_ = false;
    }
    if (this->socket_tcp_ > 0)
        ::close(this->socket_tcp_);
    if (this->ctx_.get() != nullptr)
        ::modbus_close(this->ctx_.get());
}

::maix::err::Err Slave::__receive__()
{
    this->rc_ = ::modbus_receive(this->ctx_.get(), this->query_);
    if (this->rc_ > 0) {
        if (this->debug_) {
            log::info("%s receive, len: %d", this->TAG().c_str(), this->rc_);
        }
        return ::maix::err::Err::ERR_NONE;
    }
    /* flush */
    ::modbus_flush(this->ctx_.get());
    if (this->debug_) {
        log::warn("%s receive failed", this->TAG().c_str());
    }
    return ::maix::err::Err::ERR_IO;
}

::maix::err::Err Slave::receive(const int timeout_ms)
{
    if (timeout_ms == 0) {
        this->set_timeout(0, 0);
    }else if (timeout_ms < 0) {
        this->set_timeout(std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max());
    } else {
        uint32_t sec = timeout_ms / 1000;
        uint32_t usec = timeout_ms % 1000 * 1000;
        this->set_timeout(sec, usec);
    }
    return this->__receive__();
}

RequestType Slave::request_type()
{
    return static_cast<RequestType>(this->query_[this->header_len_]);
}

::maix::err::Err Slave::reply()
{
    if (::modbus_reply(this->ctx_.get(), this->query_, this->rc_, this->mb_mapping_.get()) < 0) {
        log::warn("%s reply failed!%s", this->TAG().c_str(), ::modbus_strerror(errno));
        return ::maix::err::Err::ERR_RUNTIME;
    }
    return ::maix::err::Err::ERR_NONE;
}

RequestType Slave::receive_and_reply(const int timeout_ms)
{
    if (this->receive(timeout_ms) != ::maix::err::Err::ERR_NONE)
        return RequestType::UNKNOWN;
    auto __type__ = this->request_type();
    if (this->reply() != ::maix::err::Err::ERR_NONE)
        return RequestType::UNKNOWN;
    return __type__;
}

std::vector<uint8_t> Slave::coils(const std::vector<uint8_t>& data, const uint32_t index)
{
    const auto __get_reg_list = [this](){
        std::vector<uint8_t> __res__(this->mb_mapping_->nb_bits);
        // std::copy(this->mb_mapping_->tab_bits,
        //             this->mb_mapping_->tab_bits+this->mb_mapping_->nb_bits,
        //             __res__);
        for (int i = 0; i < this->mb_mapping_->nb_bits; ++i) {
            __res__[i] = this->mb_mapping_->tab_bits[i];
        }
        return __res__;
    };
    // read
    if (data.empty()) {
        return __get_reg_list();
    }

    if (static_cast<int>(data.size() + index) > this->mb_mapping_->nb_bits) {
        if (this->debug_)
            log::warn("%s input data out of index", this->TAG().c_str());
        return {};
    }

    // std::copy(data.begin(), data.end(), this->mb_mapping_->tab_bits+index);
    auto j = index;
    for (auto i : data) {
        this->mb_mapping_->tab_bits[j++] = i;
    }
    // return __get_reg_list();
    return {0x00};
}

std::vector<uint8_t> Slave::discrete_input(const std::vector<uint8_t>& data, const uint32_t index)
{
    const auto __get_reg_list = [this](){
        std::vector<uint8_t> __res__(this->mb_mapping_->nb_input_bits);
        // std::copy(this->mb_mapping_->tab_input_bits,
        //     this->mb_mapping_->tab_input_bits+this->mb_mapping_->nb_input_bits,
        //     __res__);
        for (int i = 0; i < this->mb_mapping_->nb_input_bits; ++i) {
            __res__[i] = this->mb_mapping_->tab_input_bits[i];
        }
        return __res__;
    };

    if (data.empty()) {
        return __get_reg_list();
    }

    if (static_cast<int>(data.size() + index) > this->mb_mapping_->nb_input_bits) {
        if (this->debug_)
            log::warn("%s input data out of index", this->TAG().c_str());
        return {};
    }

    // std::copy(data.begin(), data.end(), this->mb_mapping_->tab_input_bits+index);
    auto j = index;
    for (auto i : data) {
        this->mb_mapping_->tab_input_bits[j++] = i;
    }
    // return __get_reg_list();
    return {0x00};
}

std::vector<uint16_t> Slave::input_registers(const std::vector<uint16_t>& data, const uint32_t index)
{
    const auto __get_reg_list = [this](){
        std::vector<uint16_t> __res__(this->mb_mapping_->nb_input_registers);
        // std::copy(this->mb_mapping_->tab_input_registers,
        //     this->mb_mapping_->tab_input_registers+this->mb_mapping_->nb_input_registers,
        //     __res__);
        for (int i = 0; i < this->mb_mapping_->nb_input_registers; ++i) {
            __res__[i] = this->mb_mapping_->tab_input_registers[i];
        }
        return __res__;
    };

    if (data.empty()) {
        return __get_reg_list();
    }

    if (static_cast<int>(data.size()+index) > this->mb_mapping_->nb_input_registers) {
        if (this->debug_)
            log::warn("%s input data out of index", this->TAG().c_str());
        return {};
    }

    // std::copy(data.begin(), data.end(), this->mb_mapping_->tab_input_registers+index);
    auto j = index;
    for (auto i : data) {
        this->mb_mapping_->tab_input_registers[j++] = i;
    }
    // return __get_reg_list();
    return {0x00};
}

std::vector<uint16_t> Slave::holding_registers(const std::vector<uint16_t>& data, const uint32_t index)
{
    const auto __get_reg_list = [this]() {
        std::vector<uint16_t> __res__(this->mb_mapping_->nb_registers);
        // std::copy(this->mb_mapping_.get()->tab_registers,
        //     this->mb_mapping_->tab_registers+this->mb_mapping_->nb_registers,
        //     __res__);
        for (int i = 0; i < this->mb_mapping_->nb_registers; ++i) {
            __res__[i] = this->mb_mapping_->tab_registers[i];
        }
        return __res__;
    };

    if (data.empty()) {
        return __get_reg_list();
    }

    if (static_cast<int>(data.size()+index) > this->mb_mapping_->nb_registers) {
        if (this->debug_)
            log::warn("%s input data out of index", this->TAG().c_str());
        return {};
    }

    // std::copy(data.begin(), data.end(), this->mb_mapping_->tab_registers+index);
    auto j = index;
    for (auto i : data) {
        this->mb_mapping_->tab_registers[j++] = i;
    }
    // return __get_reg_list();
    return {0x00};
}

::modbus_mapping_t* Slave::operator->() const noexcept
{
    return this->mb_mapping_.get();
}

::modbus_t* Slave::operator()() const noexcept
{
    return this->ctx_.get();
}

/****************************** Master *********************************/

class MasterOperator final {
public:
    template<typename T>
    using ModbusOpsRFunc = std::function<int(modbus_t *ctx, int addr, int nb, T* dest)>;

    template<typename T>
    using ModbusOpsWFunc = std::function<int(modbus_t *ctx, int addr, int nb, const T* src)>;

    MasterOperator() = delete;
    MasterOperator(const MasterOperator&) = delete;
    MasterOperator(MasterOperator&&) = delete;

    MasterOperator& operator=(const MasterOperator&) = delete;
    MasterOperator& operator=(MasterOperator&&) = delete;

    ~MasterOperator() = delete;

    static const std::string TAG() noexcept;
    static void debug(bool debug) noexcept;
    static void deinit(::modbus_t* ptr) noexcept;

    static std::unique_ptr<::modbus_t, decltype(&deinit)> rtu_init(const std::string& device, int baud, int slave) noexcept;
    static std::unique_ptr<::modbus_t, decltype(&deinit)> tcp_init(const std::string& ip, int port) noexcept;

    template<typename T>
    static std::vector<T> read(::modbus_t* ctx, const uint32_t size, const uint32_t index,
                        const int timeout_ms, const std::string& name,  ModbusOpsRFunc<T> func);

    template<typename T>
    static int write(::modbus_t* ctx, const std::vector<T>& data, const uint32_t index, const int timeout_ms,
        const std::string& name, ModbusOpsWFunc<T> func);

private:
    static int debug_init(::modbus_t* ctx) noexcept;
    static int set_timeout(::modbus_t* ctx, int timeout=-1) noexcept;
    static inline std::unique_ptr<::modbus_t, decltype(&MasterOperator::deinit)> empty_ctx() noexcept;

private:
    static bool debug_;
};

bool MasterOperator::debug_ = false;

const std::string MasterOperator::TAG() noexcept
{
    return "[Maix Modbus Master]";
}

void MasterOperator::debug(bool debug) noexcept
{
    debug_ = debug;
}

std::unique_ptr<::modbus_t, decltype(&MasterOperator::deinit)> MasterOperator::empty_ctx() noexcept
{
    return std::unique_ptr<::modbus_t, decltype(&MasterOperator::deinit)>(nullptr, &MasterOperator::deinit);
}

int MasterOperator::debug_init(::modbus_t* ctx) noexcept
{
    if (::modbus_set_debug(ctx, debug_) < 0) {
        const std::string msg(TAG()+" set debug failed!"+std::string(::modbus_strerror(errno)));
        log::error(msg.c_str());
        return -1;
    }
    return 0;
}

int MasterOperator::set_timeout(::modbus_t* ctx, int timeout_ms) noexcept
{
    auto __set_timeout__ = [&ctx](uint32_t sec, uint32_t usec, const std::string& name){
        if (debug_) {
            log::info("%s timeout %s", TAG().c_str(), name.c_str());
        }
        if (::modbus_set_response_timeout(ctx, sec, usec) < 0) {
            std::string msg(TAG()+" set timeout failed! "+std::string(::modbus_strerror(errno)));
            log::warn(msg.c_str());
            return 1;
        }
        return 0;
    };

    if (timeout_ms < 0) {
        return __set_timeout__(std::numeric_limits<uint32_t>::max(), 0, "<max>");
    }

    if (timeout_ms == 0) {
        return __set_timeout__(0, 1, "<0>");
    }

    int sec = timeout_ms / 1000;
    int usec = timeout_ms % 1000 * 1000;
    std::stringstream ss;
    if (debug_)
        ss << '<' << sec << 's' << usec << "us>";

    return __set_timeout__(sec, usec, ss.str());
}

std::unique_ptr<::modbus_t, decltype(&MasterOperator::deinit)> MasterOperator::rtu_init(const std::string& device, int baud, int slave) noexcept
{
    if (debug_) {
        log::info("%s Mode: RTU, Port: %s, Baudrate: %d-8N1, Slave addr: %u.",
            TAG().c_str(), device.c_str(), baud, slave);
    }

    auto ctx = std::unique_ptr<::modbus_t, decltype(&MasterOperator::deinit)>(
        ::modbus_new_rtu(device.c_str(), baud, 'N', 8, 1),
        &MasterOperator::deinit
    );
    if (!ctx) {
        const std::string msg(TAG()+" malloc failed!");
        log::error(msg.c_str());
        return empty_ctx();
    }

    if (::modbus_set_slave(ctx.get(), slave) < 0) {
        const std::string msg(TAG()+" set slave failed!");
        log::error(msg.c_str());
        return empty_ctx();
    }

    if (debug_init(ctx.get()) < 0) {
        return empty_ctx();
    }

    if (::modbus_connect(ctx.get()) < 0) {
        const std::string msg(TAG()+" connect failed!"+std::string(::modbus_strerror(errno)));
        log::error(msg.c_str());
        return empty_ctx();
    }

    return ctx;
}

std::unique_ptr<::modbus_t, decltype(&MasterOperator::deinit)> MasterOperator::tcp_init(const std::string& ip, int port) noexcept
{
    if (debug_) {
        log::info("%s Mode: TCP, Port: %d",
            TAG().c_str(), port);
    }

    auto ctx = std::unique_ptr<::modbus_t, decltype(&MasterOperator::deinit)>(
        ::modbus_new_tcp(ip.c_str(), port),
        &MasterOperator::deinit
    );
    // auto ctx = std::make_unique<::modbus_t, decltype(&MasterOperator::deinit)>(::modbus_new_tcp(ip.c_str(), port), &deinit);
    if (!ctx) {
        const std::string msg(TAG()+" malloc failed!");
        log::error(msg.c_str());
        return empty_ctx();
    }

    if (debug_init(ctx.get()) < 0) {
        return empty_ctx();
    }

    if (::modbus_connect(ctx.get()) < 0) {
        const std::string msg(TAG()+" connect failed!"+std::string(::modbus_strerror(errno)));
        log::error(msg.c_str());
        return empty_ctx();
    }

    return ctx;
}

void MasterOperator::deinit(::modbus_t* ptr) noexcept
{
    if (ptr != nullptr) {
        ::modbus_close(ptr);
        ::modbus_free(ptr);
    }
}

template<typename T>
std::vector<T> MasterOperator::read(::modbus_t* ctx, const uint32_t size, const uint32_t index,
        const int timeout_ms, const std::string& name,  ModbusOpsRFunc<T> func)
{
    if (size == 0) {
        std::string msg(TAG()+" read length cannot be zero!");
        __throw_in_maixpy__(msg);
        return {};
    }

    set_timeout(ctx, timeout_ms);

    if (debug_) log::info("%s read %s: index<%u>, len<%u>", TAG().c_str(), name.c_str(), index, size);

    std::vector<T> __res__(size);
    int rc = func(
        ctx,
        static_cast<int>(index),
        static_cast<int>(size),
        __res__.data()
    );

    if (rc <= 0) {
        if (debug_) log::warn("%s read %s failed!", TAG().c_str(), name.c_str());
        return {};
    }

    return __res__;
}

template<typename T>
int MasterOperator::write(::modbus_t* ctx, const std::vector<T>& data, const uint32_t index, const int timeout_ms,
    const std::string& name, ModbusOpsWFunc<T> func)
{
    if (data.empty()) {
        std::string msg(TAG()+" write length cannot be zero!");
        __throw_in_maixpy__(msg);
        return -1;
    }

    set_timeout(ctx, timeout_ms);

    if (debug_) log::info("%s write %s: index<%u>, len<%u>", TAG().c_str(), name.c_str(), index, data.size());

    int rc = func(
        ctx,
        static_cast<int>(index),
        static_cast<int>(data.size()),
        data.data()
    );
    if (rc <= 0) {
        if (debug_) log::warn("%s write %s failed!", TAG().c_str(), name.c_str());
        return -1;
    }

    return rc;
}

/**************************************Master DEBUG**************************************/

void set_master_debug(bool debug)
{
    MasterOperator::debug(debug);
}

/**************************************Master RTU**************************************/

MasterRTU::MasterRTU(const std::string& device, const int baudrate)
    : device_(device), baudrate_(baudrate) {}

MasterRTU::~MasterRTU() {}

std::pair<std::string, int> MasterRTU::get_cfg(const std::string& device, const int baudrate) noexcept
{
    std::string dev = device;
    int baud = baudrate;

    if (device.empty()) {
        dev = this->device_;
    }
    if (baudrate <= 0) {
        baud = this->baudrate_;
    }

    return std::make_pair(dev, baud);
}

std::vector<uint8_t> MasterRTU::read_coils(const uint32_t slave_id, const uint32_t addr,
    const uint32_t size, const int timeout_ms, const std::string& device, const int baudrate)
{
    auto [dev, baud] = this->get_cfg(device, baudrate);
    return read_coils(dev, baud, slave_id, size, addr, timeout_ms);
}

int MasterRTU::write_coils(const uint32_t slave_id, const std::vector<uint8_t>& data,
    const uint32_t addr, const int timeout_ms, const std::string& device, const int baudrate)
{
    auto [dev, baud] = this->get_cfg(device, baudrate);
    return write_coils(dev, baud, slave_id, data, addr, timeout_ms);
}

std::vector<uint8_t> MasterRTU::read_discrete_input(const uint32_t slave_id, const uint32_t addr,
        const uint32_t size, const int timeout_ms, const std::string& device, const int baudrate)
{
    auto [dev, baud] = this->get_cfg(device, baudrate);
    return read_discrete_input(dev, baud, slave_id, size, addr, timeout_ms);
}

std::vector<uint16_t> MasterRTU::read_input_registers(const uint32_t slave_id, const uint32_t addr,
    const uint32_t size, const int timeout_ms, const std::string& device, const int baudrate)
{
    auto [dev, baud] = this->get_cfg(device, baudrate);
    return read_input_registers(dev, baud, slave_id, size, addr, timeout_ms);
}

std::vector<uint16_t> MasterRTU::read_holding_registers(const uint32_t slave_id, const uint32_t addr,
    const uint32_t size, const int timeout_ms, const std::string& device, const int baudrate)
{
    auto [dev, baud] = this->get_cfg(device, baudrate);
    return read_holding_registers(dev, baud, slave_id, size, addr, timeout_ms);
}

int MasterRTU::write_holding_registers(const uint32_t slave_id, const std::vector<uint16_t>& data,
    const uint32_t addr, const int timeout_ms, const std::string& device, const int baudrate)
{
    auto [dev, baud] = this->get_cfg(device, baudrate);
    return write_holding_registers(dev, baud, slave_id, data, addr, timeout_ms);
}

std::vector<uint8_t> MasterRTU::read_coils( const std::string& device, const int baudrate,
    const uint32_t slave_id, const uint32_t addr, const uint32_t size, const int timeout_ms)
{
    auto ctx = MasterOperator::rtu_init(device, baudrate, slave_id);
    return MasterOperator::read<uint8_t>(ctx.get(), size, addr, timeout_ms, "coils", ::modbus_read_bits);
}

int MasterRTU::write_coils(const std::string& device, const int baudrate,
    const uint32_t slave_id, const std::vector<uint8_t>& data, const uint32_t addr, const int timeout_ms)
{
    auto ctx = MasterOperator::rtu_init(device, baudrate, slave_id);
    return MasterOperator::write<uint8_t>(ctx.get(), data, addr, timeout_ms, "coils", ::modbus_write_bits);
}

std::vector<uint8_t> MasterRTU::read_discrete_input(const std::string& device, const int baudrate,
    const uint32_t slave_id, const uint32_t addr, const uint32_t size, const int timeout_ms)
{
    auto ctx = MasterOperator::rtu_init(device, baudrate, slave_id);
    return MasterOperator::read<uint8_t>(ctx.get(), size, addr, timeout_ms, "discrete input", ::modbus_read_input_bits);
}

std::vector<uint16_t> MasterRTU::read_input_registers(const std::string& device, const int baudrate,
    const uint32_t slave_id, const uint32_t addr, const uint32_t size, const int timeout_ms)
{
    auto ctx = MasterOperator::rtu_init(device, baudrate, slave_id);
    return MasterOperator::read<uint16_t>(ctx.get(), size, addr, timeout_ms, "input registers", ::modbus_read_input_registers);
}

std::vector<uint16_t> MasterRTU::read_holding_registers(const std::string& device, const int baudrate,
    const uint32_t slave_id, const uint32_t addr, const uint32_t size, const int timeout_ms)
{
    auto ctx = MasterOperator::rtu_init(device, baudrate, slave_id);
    return MasterOperator::read<uint16_t>(ctx.get(), size, addr, timeout_ms, "holding registers", ::modbus_read_registers);
}

int MasterRTU::write_holding_registers(const std::string& device, const int baudrate,
    const uint32_t slave_id, const std::vector<uint16_t>& data, const uint32_t addr, const int timeout_ms)
{
    auto ctx = MasterOperator::rtu_init(device, baudrate, slave_id);
    return MasterOperator::write<uint16_t>(ctx.get(), data, addr, timeout_ms, "holding registers", ::modbus_write_registers);
}

/**************************************Master TCP**************************************/

MasterTCP::MasterTCP(const int port) : port_(port) {}

MasterTCP::~MasterTCP() {}

int MasterTCP::get_cfg(int port) noexcept
{
    if (port < 0)
        return this->port_;
    return port;
}

std::vector<uint8_t> MasterTCP::read_coils(const std::string ip, const uint32_t addr,
    const uint32_t size, const int timeout_ms, const int port)
{
    auto p = this->get_cfg(port);
    return read_coils(ip, p, size, addr, timeout_ms);
}

int MasterTCP::write_coils(const std::string ip, const std::vector<uint8_t>& data,
                const uint32_t addr, const int timeout_ms, const int port)
{
    auto p = this->get_cfg(port);
    return write_coils(ip, p, data, addr, timeout_ms);
}

std::vector<uint8_t> MasterTCP::read_discrete_input(const std::string ip, const uint32_t addr,
    const uint32_t size, const int timeout_ms, const int port)
{
    auto p = this->get_cfg(port);
    return read_discrete_input(ip, p, size, addr, timeout_ms);
}

std::vector<uint16_t> MasterTCP::read_input_registers(const std::string ip, const uint32_t addr,
    const uint32_t size, const int timeout_ms, const int port)
{
    auto p = this->get_cfg(port);
    return read_input_registers(ip, p, size, addr, timeout_ms);
}

std::vector<uint16_t> MasterTCP::read_holding_registers(const std::string ip, const uint32_t addr,
    const uint32_t size, const int timeout_ms, const int port)
{
    auto p = this->get_cfg(port);
    return read_holding_registers(ip, p, size, addr, timeout_ms);
}

int MasterTCP::write_holding_registers(const std::string ip, const std::vector<uint16_t>& data,
    const uint32_t addr, const int timeout_ms, const int port)
{
    auto p = this->get_cfg(port);
    return write_holding_registers(ip, p, data, addr, timeout_ms);
}

std::vector<uint8_t> MasterTCP::read_coils( const std::string ip, const int port,
    const uint32_t addr, const uint32_t size, const int timeout_ms)
{
    auto ctx = MasterOperator::tcp_init(ip, port);
    return MasterOperator::read<uint8_t>(ctx.get(), size, addr, timeout_ms, "coils", ::modbus_read_bits);
}

int MasterTCP::write_coils(const std::string ip, const int port,
    const std::vector<uint8_t>& data, const uint32_t addr, const int timeout_ms)
{
    auto ctx = MasterOperator::tcp_init(ip, port);
    return MasterOperator::write<uint8_t>(ctx.get(), data, addr, timeout_ms, "coils", ::modbus_write_bits);
}

std::vector<uint8_t> MasterTCP::read_discrete_input(const std::string ip, const int port,
    const uint32_t addr, const uint32_t size, const int timeout_ms)
{
    auto ctx = MasterOperator::tcp_init(ip, port);
    return MasterOperator::read<uint8_t>(ctx.get(), size, addr, timeout_ms, "discrete input", ::modbus_read_input_bits);
}

std::vector<uint16_t> MasterTCP::read_input_registers(const std::string ip, const int port,
    const uint32_t addr, const uint32_t size, const int timeout_ms)
{
    auto ctx = MasterOperator::tcp_init(ip, port);
    return MasterOperator::read<uint16_t>(ctx.get(), size, addr, timeout_ms, "input registers", ::modbus_read_registers);
}

std::vector<uint16_t> MasterTCP::read_holding_registers(const std::string ip, const int port,
    const uint32_t addr, const uint32_t size, const int timeout_ms)
{
    auto ctx = MasterOperator::tcp_init(ip, port);
    return MasterOperator::read<uint16_t>(ctx.get(), size, addr, timeout_ms, "holding registers", ::modbus_read_registers);
}

int MasterTCP::write_holding_registers(const std::string ip, const int port,
    const std::vector<uint16_t>& data, const uint32_t addr, const int timeout_ms)
{
    auto ctx = MasterOperator::tcp_init(ip, port);
    return MasterOperator::write<uint16_t>(ctx.get(), data, addr, timeout_ms, "holding registers", ::modbus_write_registers);
}

}