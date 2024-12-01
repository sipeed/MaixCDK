/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

namespace maix
{
    /**
     * Bytes type for Python bytes compatibility.
     * @maixcdk maix.Bytes
     */
    class Bytes
    {
    public:
        /**
         * Construct a Bytes object from a uint8_t array.
         * @param data uint8_t array
         * @param len length of the array
         * @param auto_delete if true, will delete data when destruct. When copy is true, this arg will be ignore.
         * @param copy data will be copy to new buffer if true, if false, will use data directly,
         *             default true to ensure memory safety.
         * @maixcdk maix.Bytes.Bytes
         */
        Bytes(uint8_t *data, uint32_t len, bool auto_delete = false, bool copy = true)
        {
            this->data = data;
            this->data_len = len;
            this->buff_len = len;
            this->_is_alloc = auto_delete;
            if(len > 0)
            {
                if(data && copy)
                {
                    this->data = new uint8_t[this->buff_len];
                    this->_is_alloc = true;
                    memcpy(this->data, data, this->buff_len);
                }
                else if (!data && this->buff_len > 0)
                {
                    this->data = new uint8_t[this->buff_len];
                    this->_is_alloc = true;
                }
            }
        }

        /**
         * Construct a Bytes object from a uint8_t array. but not set anything.
         * @maixcdk maix.Bytes.Bytes
         */
        Bytes()
        {
            this->data = NULL;
            this->buff_len = 0;
            this->data_len = 0;
            this->_is_alloc = false;
        }

        ~Bytes()
        {
            if (_is_alloc && data)
            {
                delete[] data;
            }
        }

        /**
         * overload = operator
         * @param other another Bytes object
         * @return Bytes object
         * @maixcdk maix.Bytes.operator=
         */
        Bytes &operator=(const Bytes &other)
        {
            if (this != &other)
            {
                if (_is_alloc && data)
                {
                    delete[] data;
                }
                // alloc new buffer and copy
                this->data = new uint8_t[other.buff_len];
                this->buff_len = other.buff_len;
                this->data_len = other.data_len;
                this->_is_alloc = true;
                memcpy(this->data, other.data, this->buff_len);
            }
            return *this;
        }

        /**
         * at method
         * @param index index of the data
         * @return uint8_t data
         * @maixcdk maix.Bytes.at
         */
        uint8_t at(int index) const
        {
            if (index < 0 || index >= (int)this->data_len)
            {
                return 0;
            }
            return this->data[index];
        }

        /**
         * overload [] operator
         * @param index index of the data
         * @return uint8_t data
         * @maixcdk maix.Bytes.operator[]
         */
        uint8_t operator[](int index) const
        {
            if (index < 0 || index >= (int)this->data_len)
            {
                return 0;
            }
            return this->data[index];
        }

        /**
         * Get data length, equal to data_len.
         * @return uint32_t length
         * @maixcdk maix.Bytes.size
         */
        size_t size() const
        {
            return this->data_len;
        }

        /**
         * iter data begin
         * @maixcdk maix.Bytes.begin
         */
        uint8_t *begin()
        {
            return this->data;
        }

        /**
         * iter data end
         * @maixcdk maix.Bytes.end
         */
        uint8_t *end()
        {
            return this->data + this->data_len;
        }

        /**
         * Get raw data pointer. carefully update this value if you need change this value.
         * @return uint8_t pointer
         * @maixcdk maix.Bytes.data
         */
        uint8_t *data;

        /**
         * Length property, carefully update this value if you need change this value.
         * @return buffer length
         * @maixcdk maix.Bytes.buff_len
         */
        size_t buff_len;

        /**
         * data length, carefully update this value if you need change this value.
         * @return actual data length, <= buff_len
         * @maixcdk maix.Bytes.data_len
         */
        size_t data_len;

    private:
        bool _is_alloc;
    };

}
