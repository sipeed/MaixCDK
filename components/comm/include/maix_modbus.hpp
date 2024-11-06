#ifndef __MAIX_MODBUS_HPP__
#define __MAIX_MODBUS_HPP__

#include <cstdint>
#include <vector>
#include <memory>
#include <thread>

#include "maix_err.hpp"
#include "modbus/modbus.h"

namespace maix::comm::modbus {

    /**
     * @brief modbus mode
     * @maixpy maix.comm.modbus.Mode
     */
    enum class Mode : unsigned char {
        RTU,
        TCP
    };


    /**
     * @brief Modbus request types enumeration.
     *
     * This enumeration defines the various Modbus request types,
     * including functions for reading and writing coils, registers,
     * as well as diagnostics and identification.
     *
     * @maixpy maix.comm.modbus.RequestType
     */
    enum class RequestType : unsigned char {
        READ_COILS                      = 0x01,     ///< Read Coils
        READ_DISCRETE_INPUTS            = 0x02,     ///< Read Discrete Inputs
        READ_HOLDING_REGISTERS          = 0x03,     ///< Read Holding Registers
        READ_INPUT_REGISTERS            = 0x04,     ///< Read Input Registers
        WRITE_SINGLE_COIL               = 0x05,     ///< Write Single Coil
        WRITE_SINGLE_REGISTER           = 0x06,     ///< Write Single Register
        DIAGNOSTICS                     = 0x08,     ///< Diagnostics (Serial Line only)
        GET_COMM_EVENT_COUNTER          = 0x0B,     ///< Get Comm Event Counter (Serial Line only)
        WRITE_MULTIPLE_COILS            = 0x0F,     ///< Write Multiple Coils
        WRITE_MULTIPLE_REGISTERS        = 0x10,     ///< Write Multiple Registers
        REPORT_SERVER_ID                = 0x11,     ///< Report Slave ID (Serial Line only)
        MASK_WRITE_REGISTER             = 0x16,     ///< Mask Write Register
        READ_WRITE_MULTIPLE_REGISTERS   = 0x17,     ///< Read/Write Multiple Registers
        READ_DEVICE_IDENTIFICATION      = 0x2B,     ///< Read Device Identification
        UNKNOWN                         = 0xFF      ///< Unknown Request Type
    };

    /**
     * @brief Structure to hold information about a Modbus register.
     *
     * This structure contains the starting address and size of a Modbus register,
     * allowing for easy management of register information.
     *
     * @maixcdk maix.comm.modbus.RegisterInfo
     */
    struct RegisterInfo {
        uint32_t start_address;  ///< Starting address of the register
        uint32_t size;          ///< Size of the register (number of elements)

        /**
         * @brief Constructor to initialize RegisterInfo.
         *
         * @param start_addr The starting address of the register.
         * @param sz The size of the register as the number of elements.
         */
        RegisterInfo(uint32_t start_addr, uint32_t sz)
            : start_address(start_addr), size(sz) {}
        RegisterInfo() : start_address(0), size(0) {}
    };

    /**
     * @brief Structure to hold information about multiple Modbus registers.
     *
     * This structure aggregates information for various types of Modbus registers,
     * including coils, discrete inputs, holding registers, and input registers.
     *
     * @maixcdk maix.comm.modbus.Registers
     */
    struct Registers {
        RegisterInfo coils;                ///< Information for coils
        RegisterInfo discrete_inputs;        ///< Information for discrete inputs
        RegisterInfo holding_registers;      ///< Information for holding registers
        RegisterInfo input_registers;        ///< Information for input registers

        /**
         * @brief Constructor to initialize all register information.
         *
         * @param coils_start The starting address of the coils register.
         * @param coils_size The size of the coils register.
         * @param discrete_start The starting address of the discrete inputs register.
         * @param discrete_size The size of the discrete inputs register.
         * @param holding_start The starting address of the holding registers.
         * @param holding_size The size of the holding registers.
         * @param input_start The starting address of the input registers.
         * @param input_size The size of the input registers.
         */
        Registers(uint32_t coils_start, uint32_t coils_size,
                uint32_t discrete_start, uint32_t discrete_size,
                uint32_t holding_start, uint32_t holding_size,
                uint32_t input_start, uint32_t input_size)
            : coils(coils_start, coils_size),
            discrete_inputs(discrete_start, discrete_size),
            holding_registers(holding_start, holding_size),
            input_registers(input_start, input_size) {}
        Registers() {}
    };

    /**
     * Class for modbus Slave
     * @maixpy maix.comm.modbus.Slave
     */
    class Slave {
    public:
        /**
         * @brief Modbus Slave constructor.
         *
         * This constructor initializes a Modbus Slave instance. Depending on the mode (RTU or TCP),
         * it sets up the necessary parameters for communication and defines the register structure.
         *
         * @param mode Specifies the communication mode: RTU or TCP.
         *             @see modbus.Mode for valid modes.
         * @param ip_or_device The UART device name if using RTU mode.
         *                     If TCP mode is selected, this parameter is ignored.
         * @param coils_start The starting address of the coils register.
         * @param coils_size The number of coils to manage.
         * @param discrete_start The starting address of the discrete inputs register.
         * @param discrete_size The number of discrete inputs to manage.
         * @param holding_start The starting address of the holding registers.
         * @param holding_size The number of holding registers to manage.
         * @param input_start The starting address of the input registers.
         * @param input_size The number of input registers to manage.
         * @param rtu_baud The baud rate for RTU communication.
         *                 Supported rates include: 110, 300, 600, 1200, 2400, 4800,
         *                 9600, 19200, 38400, 57600, 115200, 230400, 460800,
         *                 500000, 576000, 921600, 1000000, 1152000, 1500000,
         *                 2500000, 3000000, 3500000, 4000000.
         *                 Default is 115200. Ensure that the selected baud rate
         *                 is supported by the underlying hardware and libmodbus.
         * @param rtu_slave The RTU slave address. Ignored in TCP mode. Default is 1.
         * @param tcp_port The port used for TCP communication. Ignored in RTU mode. Default is 5020.
         * @param debug A boolean flag to enable or disable debug mode. Default is false.
         *
         * @maixpy maix.comm.modbus.Slave.__init__
         */
        Slave(maix::comm::modbus::Mode mode, const std::string& ip_or_device,
            uint32_t coils_start=0, uint32_t coils_size=0,
            uint32_t discrete_start=0, uint32_t discrete_size=0,
            uint32_t holding_start=0, uint32_t holding_size=0,
            uint32_t input_start=0, uint32_t input_size=0,
            int rtu_baud=115200, uint8_t rtu_slave=1,
            int tcp_port=5020, bool debug=false);

        /**
         * @brief Constructor for creating a Modbus Slave instance with specified registers.
         *
         * This constructor initializes a Modbus Slave using the provided mode and connection
         * parameters. It also allows for defining register information through a
         * `Registers` object, enabling easier management of multiple registers at once.
         *
         * @param mode Specifies the communication mode: RTU or TCP.
         *             @see modbus.Mode for valid modes.
         * @param ip_or_device The UART device name if using RTU mode.
         *                     If TCP mode is chosen, this parameter is ignored.
         * @param registers A Registers object that holds starting addresses and sizes
         *                  for coils, discrete inputs, holding registers, and input registers.
         *                  This allows the user to set up the Slave's register configuration
         *                  in a structured manner.
         * @param rtu_baud The baud rate for RTU communication.
         *                 Supported rates include: 110, 300, 600, 1200, 2400, 4800,
         *                 9600, 19200, 38400, 57600, 115200, 230400, 460800,
         *                 500000, 576000, 921600, 1000000, 1152000, 1500000,
         *                 2500000, 3000000, 3500000, 4000000.
         *                 Default is 115200. Ensure that the selected baud rate
         *                 is supported by the underlying hardware and libmodbus.
         * @param rtu_slave The RTU slave address. Ignored in TCP mode. Default is 1.
         * @param tcp_port The port used for TCP communication. Ignored in RTU mode. Default is 5020.
         * @param debug A boolean flag to enable or disable debug mode. Default is false.
         *
         * @maixcdk maix.modbus.Slave.__init__
         */
        Slave(maix::comm::modbus::Mode mode, const std::string& ip_or_device,
            const Registers& registers=Registers{},
            int rtu_baud=115200, uint8_t rtu_slave=1,
            int tcp_port=5020, bool debug=false);


        ~Slave();

        Slave(const Slave&) = delete;
        Slave& operator=(const Slave&) = delete;
        Slave(Slave&& other) = delete;
        Slave& operator=(Slave&& other) = delete;

        /**
         * @brief Receives a Modbus request
         *
         * This function is used to receive a Modbus request from the client. The behavior of the function
         * depends on the parameter `timeout_ms` provided, which dictates how long the function will wait
         * for a request before returning.
         *
         * @note This function gets and parses the request, it does not manipulate the registers.
         *
         * @param timeout_ms Timeout setting
         *        -1   Block indefinitely until a request is received
         *        0    Non-blocking mode; function returns immediately, regardless of whether a request is received
         *        >0   Blocking mode; function will wait for the specified number of milliseconds for a request
         *
         * @return maix::err::Err type, @see maix::err::Err
         *
         * @maixpy maix.comm.modbus.Slave.receive
         */
        ::maix::err::Err receive(const int timeout_ms=-1);

        /**
         * @brief Gets the type of the Modbus request that was successfully received
         *
         * This function can be used to retrieve the type of the request received after a successful
         * call to `receive()`. The return value indicates the type of the Modbus request, allowing
         * the user to understand and process the received request appropriately.
         *
         * @return RequestType The type of the Modbus request that has been received.
         *          @see modbus.RequestType
         *
         * @maixpy maix.comm.modbus.Slave.request_type
         */
        ::maix::comm::modbus::RequestType request_type();

        /**
         * @brief Processes the request and returns the corresponding data.
         *
         * This function handles the requests received from the client. It retrieves any data that the client
         * needs to write to the registers and updates the registers accordingly. Additionally, it retrieves
         * the requested data from the registers and sends it back to the client in the response.
         *
         * This function is essential for managing read and write operations in a Modbus Slave context.
         *
         * @note The function will modify the Slave's internal state based on the data received in the
         *       request, and ensure that the appropriate data is returned to the client.
         *
         * @return maix::err::Err type, @see maix::err::Err
         *
         * @maixpy maix.comm.modbus.Slave.reply
         */
        ::maix::err::Err reply();

        /**
         * @brief Receives a request from the client and sends a response.
         *
         * This function combines the operations of receiving a request and sending a corresponding
         * response in a single call. It waits for a specified duration (defined by the `timeout_ms`
         * parameter) to receive a request from the client. Upon successful receipt of the request,
         * it processes the request and prepares the necessary data to be sent back to the client.
         *
         * @param timeout_ms The timeout duration for waiting to receive a request.
         *                   - A value of -1 makes the function block indefinitely until a request
         *                     is received.
         *                   - A value of 0 makes it non-blocking, returning immediately without
         *                     waiting for a request.
         *                   - A positive value specifies the maximum time (in milliseconds) to wait
         *                     for a request before timing out.
         *
         * @return RequestType The type of the Modbus request that has been received.
         *          @see modbus.RequestType
         *
         * @maixpy maix.comm.modbus.Slave.receive_and_reply
         */
        ::maix::comm::modbus::RequestType receive_and_reply(const int timeout_ms=-1);

        /**
         * @brief Reads from or writes to coils.
         *
         * This function can be used to either read data from coils or write data to them.
         * If the `data` parameter is empty, the function performs a read operation.
         * If `data` is not empty, the function writes the contents of `data` to the coils
         * starting at the specified index.
         *
         * @param data A vector of data to be written. If empty, a read operation is performed.
         *             If not empty, the data will overwrite the coils from `index`.
         * @param index The starting index for writing data. This parameter is ignored during read operations.
         *
         * @return std::vector<uint16_t> When the read operation is successful, return all data in the coils as a list.
         *         When the write operation is successful, return a non-empty list; when it fails, return an empty list.
         *
         * @maixpy maix.comm.modbus.Slave.coils
         */
        std::vector<uint8_t> coils(const std::vector<uint8_t>& data = std::vector<uint8_t>{}, const uint32_t index = 0);

        /**
         * @brief Reads from or writes to discrete input.
         *
         * This function can be used to either read data from discrete input or write data to them.
         * If the `data` parameter is empty, the function performs a read operation.
         * If `data` is not empty, the function writes the contents of `data` to the discrete input
         * starting at the specified index.
         *
         * @param data A vector of data to be written. If empty, a read operation is performed.
         *             If not empty, the data will overwrite the discrete input from `index`.
         * @param index The starting index for writing data. This parameter is ignored during read operations.
         *
         * @return std::vector<uint16_t> When the read operation is successful, return all data in the discrete input as a list.
         *         When the write operation is successful, return a non-empty list; when it fails, return an empty list.
         *
         * @maixpy maix.comm.modbus.Slave.discrete_input
         */
        std::vector<uint8_t> discrete_input(const std::vector<uint8_t>& data = std::vector<uint8_t>{}, const uint32_t index = 0);

        /**
         * @brief Reads from or writes to input registers.
         *
         * This function can be used to either read data from input registers or write data to them.
         * If the `data` parameter is empty, the function performs a read operation.
         * If `data` is not empty, the function writes the contents of `data` to the input registers
         * starting at the specified index.
         *
         * @param data A vector of data to be written. If empty, a read operation is performed.
         *             If not empty, the data will overwrite the input registers from `index`.
         * @param index The starting index for writing data. This parameter is ignored during read operations.
         *
         * @return std::vector<uint16_t> When the read operation is successful, return all data in the input registers as a list.
         *         When the write operation is successful, return a non-empty list; when it fails, return an empty list.
         *
         * @maixpy maix.comm.modbus.Slave.input_registers
         */
        std::vector<uint16_t> input_registers(const std::vector<uint16_t>& data = std::vector<uint16_t>{}, const uint32_t index = 0);

        /**
         * @brief Reads from or writes to holding registers.
         *
         * This function can be used to either read data from holding registers or write data to them.
         * If the `data` parameter is empty, the function performs a read operation.
         * If `data` is not empty, the function writes the contents of `data` to the holding registers
         * starting at the specified index.
         *
         * @param data A vector of data to be written. If empty, a read operation is performed.
         *             If not empty, the data will overwrite the holding registers from `index`.
         * @param index The starting index for writing data. This parameter is ignored during read operations.
         *
         * @return std::vector<uint16_t> When the read operation is successful, return all data in the holding registers as a list.
         *         When the write operation is successful, return a non-empty list; when it fails, return an empty list.
         *
         * @maixpy maix.comm.modbus.Slave.holding_registers
         */
        std::vector<uint16_t> holding_registers(const std::vector<uint16_t>& data = std::vector<uint16_t>{}, const uint32_t index = 0);

        /**
         * @brief Returns the raw pointer to the modbus_mapping_t, allowing direct manipulation of registers to avoid copy overhead.
         *
         * @note: The returned pointer must not be deleted or freed.
         *
         * @return ::modbus_mapping_t* type
         */
        ::modbus_mapping_t* operator->();

    private:
        void rtu_init(const std::string& device, int baud, uint8_t slave);
        void tcp_init(const std::string& ip, int port);
        const std::string TAG() const noexcept;
        void header_init();
        void debug_init();
        void mapping_init();
        ::maix::err::Err __receive__();
        ::maix::err::Err set_timeout(uint32_t sec, uint32_t usec);
    private:
        std::unique_ptr<modbus_t, decltype(&modbus_free)>
            ctx_{nullptr, &modbus_free};
        std::unique_ptr<modbus_mapping_t, decltype(&modbus_mapping_free)>
            mb_mapping_{nullptr, &modbus_mapping_free};
        Registers registers_info_;
        bool debug_;
        int rc_{0};
        int header_len_;
        int socket_tcp_{-1};
        uint8_t query_[128]{0};
        uint32_t curr_timeout_sec_{165};
        uint32_t curr_timeout_usec_{528};
        std::unique_ptr<std::thread> tcp_listener_{nullptr};
        bool tcp_listener_need_exit_{false};
    };
}



#endif // __MAIX_MODBUS_HPP__