/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include <string>

namespace maix::nn
{
    /**
     * Object for detect result
     * @maixpy maix.nn.Object
    */
    class Object
    {
    public:
        /**
         * Constructor of Object for detect result
         * @param x left top x
         * @param y left top y
         * @param w width
         * @param h height
         * @param class_id class id
         * @param score score
         * @maixpy maix.nn.Object.__init__
         * @maixcdk maix.nn.Object.Object
        */
        Object(int x = 0, int y = 0, int w = 0, int h = 0, int class_id = 0, float score = 0)
            : x(x), y(y), w(w), h(h), class_id(class_id), score(score)
        {
        }

        ~Object()
        {
        }

        /**
         * Object info to string
         * @return Object info string
         * @maixpy maix.nn.Object.__str__
         * @maixcdk maix.nn.Object.to_str
        */
        std::string to_str()
        {
            return "x: " + std::to_string(x) + ", y: " + std::to_string(y) + ", w: " + std::to_string(w) + ", h: " + std::to_string(h) + ", class_id: " + std::to_string(class_id) + ", score: " + std::to_string(score);
        }

        /**
         * Object left top coordinate x
         * @maixpy maix.nn.Object.x
        */
        int x;

        /**
         * Object left top coordinate y
         * @maixpy maix.nn.Object.y
        */
        int y;

        /**
         * Object width
         * @maixpy maix.nn.Object.w
        */
        int w;

        /**
         * Object height
         * @maixpy maix.nn.Object.h
        */
        int h;

        /**
         * Object class id
         * @maixpy maix.nn.Object.class_id
        */
        int class_id;

        /**
         * Object score
         * @maixpy maix.nn.Object.score
        */
        float score;
    };
}

