/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2025.7.1: Add framework, create this file.
 */

#pragma once

#include "maix_log.hpp"
#include "maix_err.hpp"
#include "maix_image.hpp"
#include <stdlib.h>
#include <stdexcept>

/**
 * @brief maix.pipeline module, video stream processing via pipeline
 * @maixpy maix.pipeline
*/
namespace maix::pipeline
{
    /**
     * Stream class, saved the video stream data
     * @maixpy maix.pipeline.Stream
    */
    class Stream
    {
    public:
        /**
         * @brief Construct a new Stream object
         * @param stream stream handle
         * @param auto_delete auto delete stream when object is destroyed
         * @param from When releasing the object, the 'from' parameter will be referenced to determine the release method.
         * @maixcdk maix.pipeline.Stream.Stream
        */
        Stream(void *stream, bool auto_delete = false, std::string from = "");
        ~Stream();

        /**
         * @brief Since a single stream may contain multiple pieces of data, this returns the number of data segments present.
         * @maixpy maix.pipeline.Stream.data_count
        */
        int data_count();

        /**
         * @brief Get the data stream at index
         * @param idx data index, must be less than data_count().
         * @return Returns the data at index. Note: when using C++, you need to manually release the memory.
         * @maixpy maix.pipeline.Stream.data
        */
        Bytes *data(int idx);

        /**
         * @brief Get the data size at index
         * @param idx data index, must be less than data_count().
         * @return Returns the data size at index.
         * @maixpy maix.pipeline.Stream.data_size
        */
        int data_size(int idx);

        /**
         * @brief Get the SPS frame data; if the frame does not exist, return null.
         * @return SPS frame data.
         * @maixpy maix.pipeline.Stream.get_sps_frame
        */
        Bytes *get_sps_frame();

        /**
         * @brief Get the PPS frame data; if the frame does not exist, return null.
         * @return PPS frame data.
         * @maixpy maix.pipeline.Stream.get_pps_frame
        */
        Bytes *get_pps_frame();

        /**
         * @brief Get the I frame data; if the frame does not exist, return null.
         * @return I frame data.
         * @maixpy maix.pipeline.Stream.get_i_frame
        */
        Bytes *get_i_frame();

        /**
         * Get the PTS(Presentation Timestamp) of the stream.
         * @return P frame data.
         * @maixpy maix.pipeline.Stream.get_p_frame
        */
        Bytes *get_p_frame();

        /**
         * @brief Check if the stream has PPS frame.
         * @return PPS frame data.
         * @maixpy maix.pipeline.Stream.has_pps_frame
        */
        bool has_pps_frame();

        /**
         * @brief Check if the stream has SPS frame.
         * @return SPS frame data.
         * @maixpy maix.pipeline.Stream.has_sps_frame
        */
        bool has_sps_frame();

        /**
         * @brief Check if the stream has I frame.
         * @return True if the stream has I frame, otherwise false.
         * @maixpy maix.pipeline.Stream.has_i_frame
        */
        bool has_i_frame();

        /**
         * @brief Check if the stream has P frame.
         * @return True if the stream has P frame, otherwise false.
         * @maixpy maix.pipeline.Stream.has_p_frame
        */
        bool has_p_frame();

        /**
         * @brief Get the pts(Presentation Timestamp) of the stream
         * @return Returns the pts of the stream.
         * @maixpy maix.pipeline.Stream.pts
        */
        int pts();

        /**
         * @brief Get frame
         * @return Returns the frame.
         * @maixcdk maix.pipeline.Stream.stream
        */
        void *stream();
    private:
        void *__stream;
        bool __auto_delete;
        std::string __from;
    };

    /**
     * Frame class, saved the image data
     * @maixpy maix.pipeline.Frame
    */
    class Frame
    {
    public:
        /**
         * @brief Construct a new Frame object
         * @param frame frame handle
         * @param auto_delete auto delete frame when object is destroyed
         * @param from When releasing the object, the 'from' parameter will be referenced to determine the release method.
         * @maixpy maix.pipeline.Frame.__init__
         * @maixcdk maix.pipeline.Frame.Frame
        */
        Frame(void *frame, bool auto_delete = false, std::string from = "");
        ~Frame();

        /**
         * @brief Get the width of the frame
         * @return Returns the width of the frame.
         * @maixpy maix.pipeline.Frame.width
        */
        int width();

        /**
         * @brief Get the height of the frame
         * @return Returns the height of the frame.
         * @maixpy maix.pipeline.Frame.height
        */
        int height();

        /**
         * @brief Get the format of the frame
         * @return Returns the format of the frame.
         * @maixpy maix.pipeline.Frame.format
        */
        image::Format format();

        /**
         * @brief Convert the frame to an image
         * @return Returns an image object.
         * @maixpy maix.pipeline.Frame.to_image
        */
        image::Image *to_image();

        /**
         * @brief Get the stride of the plane. Stride represents the number of bytes occupied in memory by each row of image data.
         * It is usually greater than or equal to the number of bytes actually used by the pixels in that row.
         * In image processing, different image formats are divided into multiple planes.
         * Typically, RGB images have only one valid plane, while NV21/NV12 images have two valid planes.
         * @param idx plane index.
         * @return Returns the stride of the frame.
         * @maixpy maix.pipeline.Frame.stride
        */
        int stride(int idx);

        /**
         * @brief Get the virtual address of the plane. In image processing, different image formats are divided into multiple planes.
         * Typically, RGB images have only one valid plane, while NV21/NV12 images have two valid planes.
         * @note You can read image data from this address, but you need to be very careful.
         * If the current object has been released, operating on this address is prohibited.
         * @param idx plane index.
         * @return Returns the virtual address of the frame.
         * @maixpy maix.pipeline.Frame.virtual_address
        */
        uint64_t virtual_address(int idx);

        /**
         * @brief Get the physical address of the plane. In image processing, different image formats are divided into multiple planes.
         * Typically, RGB images have only one valid plane, while NV21/NV12 images have two valid planes.
         * @note Don’t operate on this address.
         * @param idx plane index.
         * @return Returns the physical address of the frame.
         * @maixpy maix.pipeline.Frame.physical_address
        */
        uint64_t physical_address(int idx);

        /**
         * @brief Get frame
         * @return Returns the frame.
         * @maixcdk maix.pipeline.Frame.frame
        */
        void *frame();
    private:
        void *__frame;
        bool __auto_delete;
        std::string __from;
    };
}
