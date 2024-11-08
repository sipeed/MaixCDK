#include "maix_modbus.hpp"
#include "maix_log.hpp"

#include <cstring> // std::memcpy
#include <stdexcept> // std::runtime_error
#include <unistd.h> // close
#include <limits>
#include <sys/select.h> // select

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
    if (this->rc_ >= 0) {
        if (this->debug_) {
            log::info("%s receive, len: %d", this->TAG().c_str(), this->rc_);
        }
        return ::maix::err::Err::ERR_NONE;
    }
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

const std::string Master::TAG() const noexcept
{
    return "[Maix Modbus Master]";
}

void Master::rtu_init(const std::string& device, int baud, uint8_t slave)
{
    if (this->debug_) {
        log::info("%s Mode: RTU, Port: %s, Baudrate: %d-8N1, Slave addr: %u.",
            this->TAG().c_str(), device.c_str(), baud, slave);
    }

    this->ctx_.reset(::modbus_new_rtu(device.c_str(), baud, 'N', 8, 1));
    if (this->ctx_ == nullptr) {
        const std::string msg(this->TAG()+" malloc failed!");
        __error_and_throw__(msg);
    }

    if (::modbus_set_slave(this->ctx_.get(), slave) < 0) {
        const std::string msg(this->TAG()+" set slave failed!");
        __error_and_throw__(msg);
    }

    this->debug_init();

    if (::modbus_connect(this->ctx_.get()) < 0) {
        const std::string msg(this->TAG()+" connect failed!"+std::string(::modbus_strerror(errno)));
        __error_and_throw__(msg);
    }
}

void Master::tcp_init(const std::string& ip, int port)
{
    if (this->debug_) {
        log::info("%s Mode: TCP, Port: %d",
            this->TAG().c_str(), port);
    }

    this->ctx_.reset(::modbus_new_tcp(ip.c_str(), port));
    if (this->ctx_.get() == nullptr) {
        const std::string msg(this->TAG()+" malloc failed!");
        __error_and_throw__(msg);
    }

    this->debug_init();

    if (::modbus_connect(this->ctx_.get()) < 0) {
        const std::string msg(this->TAG()+" connect failed!"+std::string(::modbus_strerror(errno)));
        __error_and_throw__(msg);
    }
}

void Master::debug_init()
{
    if (::modbus_set_debug(this->ctx_.get(), this->debug_) < 0) {
        const std::string msg(this->TAG()+" set debug failed!"+std::string(::modbus_strerror(errno)));
        __error_and_throw__(msg);
    }
    if (this->debug_) {
        log::info("%s set debug succ", this->TAG().c_str());
    }
}

::maix::err::Err Master::set_timeout(uint32_t sec, uint32_t usec)
{
    // modbus_set_response_timeout
    // nonblocking
    if (sec == 0 && usec == 0) {
        if (this->curr_timeout_sec_ == 0 && this->curr_timeout_usec_ == 1)
            return ::maix::err::Err::ERR_NONE;
        if (this->debug_) {
            log::info("%s Timeout: 0", this->TAG().c_str());
        }
        this->curr_timeout_sec_ = 0;
        this->curr_timeout_usec_ = 1;
        if (::modbus_set_response_timeout(this->ctx_.get(), this->curr_timeout_sec_, this->curr_timeout_usec_) < 0) {
            std::string msg(this->TAG()+" set timeout failed!"+std::string(::modbus_strerror(errno)));
            log::warn(msg.c_str());
            return ::maix::err::Err::ERR_RUNTIME;
            // __error_and_throw__(msg);
        }
        return ::maix::err::Err::ERR_NONE;
    }

    // blocking
    if (usec > 999999 || sec == std::numeric_limits<uint32_t>::max()) {
        if (this->curr_timeout_sec_ == std::numeric_limits<uint32_t>::max() && this->curr_timeout_usec_ == 999999)
            return ::maix::err::Err::ERR_NONE;
        if (this->debug_) {
            log::info("%s Timeout: max", this->TAG().c_str());
        }
        this->curr_timeout_sec_ = std::numeric_limits<uint32_t>::max();
        this->curr_timeout_usec_ = 999999;
        if (::modbus_set_response_timeout(this->ctx_.get(), this->curr_timeout_sec_, this->curr_timeout_usec_) < 0) {
            std::string msg(this->TAG()+" set timeout failed!"+std::string(::modbus_strerror(errno)));
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
            this->TAG().c_str(), sec, usec);
    }
    this->curr_timeout_sec_ = sec;
    this->curr_timeout_usec_ = usec;
    if (::modbus_set_response_timeout(this->ctx_.get(), this->curr_timeout_sec_, this->curr_timeout_usec_) < 0) {
        std::string msg(this->TAG()+" set timeout failed!"+std::string(::modbus_strerror(errno)));
        log::warn(msg.c_str());
        return ::maix::err::Err::ERR_RUNTIME;
        // __error_and_throw__(msg);
    }
    return ::maix::err::Err::ERR_NONE;
}

Master::Master(::maix::comm::modbus::Mode mode, const std::string& ip_or_device,
                int rtu_baud, uint8_t rtu_slave, int tcp_port, bool debug) : debug_(debug)
{
    if (mode == Mode::RTU)
        this->rtu_init(ip_or_device, rtu_baud, rtu_slave);
    else if (mode == Mode::TCP)
        this->tcp_init(ip_or_device, tcp_port);
    else
        __error_and_throw__(this->TAG()+" unknown Mode!");

    if (this->set_timeout(
                std::numeric_limits<uint32_t>::max(),
                std::numeric_limits<uint32_t>::max()
            ) != ::maix::err::Err::ERR_NONE) {
        const std::string errmsg(this->TAG()+" Set timeout failed");
        log::error(errmsg.c_str());
        throw std::runtime_error(errmsg);
    }
}

Master::Master(Master&& other) noexcept
    :   ctx_(std::move(other.ctx_)), debug_(other.debug_),
        curr_timeout_sec_(other.curr_timeout_sec_),
        curr_timeout_usec_(other.curr_timeout_usec_)
{
    other.debug_ = false;
    other.curr_timeout_sec_ = 165;
    other.curr_timeout_usec_ = 528;
}

Master& Master::operator=(Master&& other) noexcept
{
    this->ctx_ = std::move(other.ctx_);
    this->debug_ = other.debug_;
    this->curr_timeout_sec_ = other.curr_timeout_sec_;
    this->curr_timeout_usec_ = other.curr_timeout_usec_;

    other.debug_ = false;
    other.curr_timeout_sec_ = 165;
    other.curr_timeout_usec_ = 528;

    return *this;
}

Master::~Master()
{
    if (this->ctx_ != nullptr)
        ::modbus_close(this->ctx_.get());
}

template<typename T>
std::vector<T> Master::read(const uint32_t size, const uint32_t index, const int timeout_ms,
                            const std::string& name, std::function<int(modbus_t *ctx, int addr, int nb, T* dest)> func)
{
    if (size == 0) {
        std::string msg(this->TAG()+" read length cannot be zero!");
        __throw_in_maixpy__(msg);
        return {};
    }

    if (timeout_ms == 0) {
        this->set_timeout(0, 0);
    }else if (timeout_ms < 0) {
        this->set_timeout(std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max());
    } else {
        uint32_t sec = timeout_ms / 1000;
        uint32_t usec = timeout_ms % 1000 * 1000;
        this->set_timeout(sec, usec);
    }

    if (this->debug_) log::info("%s read %s: index<%u>, len<%u>", this->TAG().c_str(), name.c_str(), index, size);
    std::vector<T> __res__(size);
    int rc = func(
        this->ctx_.get(),
        static_cast<int>(index),
        static_cast<int>(size),
        __res__.data()
    );

    if (rc <= 0) {
        if (this->debug_) log::warn("%s read %s failed!", this->TAG().c_str(), name.c_str());
        return {};
    }

    return __res__;
}

template<typename T>
int Master::write(const std::vector<T>& data, const uint32_t index, const int timeout_ms,
    const std::string& name, std::function<int(modbus_t *ctx, int addr, int nb, const T* src)> func)
{
    if (data.empty()) {
        std::string msg(this->TAG()+" read length cannot be zero!");
        __throw_in_maixpy__(msg);
        return -1;
    }

    if (timeout_ms == 0) {
        this->set_timeout(0, 0);
    }else if (timeout_ms < 0) {
        this->set_timeout(std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max());
    } else {
        uint32_t sec = timeout_ms / 1000;
        uint32_t usec = timeout_ms % 1000 * 1000;
        this->set_timeout(sec, usec);
    }

    if (this->debug_) log::info("%s write %s: index<%u>, len<%llu>", this->TAG().c_str(), name.c_str(), index, data.size());

    int rc = func(
        this->ctx_.get(),
        static_cast<int>(index),
        static_cast<int>(data.size()),
        data.data()
    );
    if (rc <= 0) {
        if (this->debug_) log::warn("%s write %s failed!", this->TAG().c_str(), name.c_str());
        return -1;
    }

    return rc;
}

std::vector<uint8_t> Master::read_coils(const uint32_t size, const uint32_t index, const int timeout_ms)
{
    return this->read<uint8_t>(size, index, timeout_ms, "coils", ::modbus_read_bits);
}

std::vector<uint8_t> Master::read_discrete_input(const uint32_t size, const uint32_t index, const int timeout_ms)
{
    return this->read<uint8_t>(size, index, timeout_ms, "discrete input", ::modbus_read_input_bits);
}

std::vector<uint16_t> Master::read_input_registers(const uint32_t size, const uint32_t index, const int timeout_ms)
{
    return this->read<uint16_t>(size, index, timeout_ms, "input registers", ::modbus_read_input_registers);
}

std::vector<uint16_t> Master::read_holding_registers(const uint32_t size, const uint32_t index, const int timeout_ms)
{
    return this->read<uint16_t>(size, index, timeout_ms, "holding registers", ::modbus_read_registers);
}

int Master::write_coils(const std::vector<uint8_t>& data, const uint32_t index, const int timeout_ms)
{
    return this->write<uint8_t>(data, index, timeout_ms, "coils", ::modbus_write_bits);
}

int Master::write_holding_registers(const std::vector<uint16_t>& data, const uint32_t index, const int timeout_ms)
{
    return this->write<uint16_t>(data, index, timeout_ms, "holding registers", ::modbus_write_registers);
}

::modbus_t* Master::context() const noexcept
{
    return this->ctx_.get();
}

::modbus_t* Master::operator()() const noexcept
{
    return this->context();
}

}