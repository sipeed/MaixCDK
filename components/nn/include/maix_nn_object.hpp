/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include <string>
#include <vector>
#include "maix_image.hpp"

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
        Object(int x = 0, int y = 0, int w = 0, int h = 0, int class_id = 0, float score = 0, std::vector<int> points = std::vector<int>(), float angle = -9999)
            : x(x), y(y), w(w), h(h), class_id(class_id), score(score), points(points), angle(angle), seg_mask(NULL), temp(NULL)
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
            if(angle != -9999)
                return "x: " + std::to_string(x) + ", y: " + std::to_string(y) + ", w: " + std::to_string(w) + ", h: " + std::to_string(h) + ", class_id: " + std::to_string(class_id) + ", score: " + std::to_string(score) + ", angle: " + std::to_string(angle);
            else
                return "x: " + std::to_string(x) + ", y: " + std::to_string(y) + ", w: " + std::to_string(w) + ", h: " + std::to_string(h) + ", class_id: " + std::to_string(class_id) + ", score: " + std::to_string(score);
        }

        /**
         * Get OBB(oriented bounding box) points, auto calculated according to x,y,w,h,angle
         * @maixpy maix.nn.Object.get_obb_points
         */
        std::vector<int> get_obb_points()
        {
            // x, y, w, h, angle to x1,y1,x2,y2,x3,y3,x4,y4 list
            std::vector<int> obb_points;
            if(angle == -9999)
            {
                obb_points.push_back(x);
                obb_points.push_back(y);
                obb_points.push_back(x + w);
                obb_points.push_back(y);
                obb_points.push_back(x + w);
                obb_points.push_back(y + h);
                obb_points.push_back(x);
                obb_points.push_back(y + h);
            }
            else
            {
                float angle_rad = angle * M_PI;
                float c = cosf(angle_rad);
                float s = sinf(angle_rad);
                int x1 = x;
                int y1 = y;
                int x2 = x + w;
                int y2 = y;
                int x3 = x + w;
                int y3 = y + h;
                int x4 = x;
                int y4 = y + h;
                int cx = x + w / 2;
                int cy = y + h / 2;
                obb_points.push_back((int)(c * (x1 - cx) - s * (y1 - cy) + cx));
                obb_points.push_back((int)(s * (x1 - cx) + c * (y1 - cy) + cy));
                obb_points.push_back((int)(c * (x2 - cx) - s * (y2 - cy) + cx));
                obb_points.push_back((int)(s * (x2 - cx) + c * (y2 - cy) + cy));
                obb_points.push_back((int)(c * (x3 - cx) - s * (y3 - cy) + cx));
                obb_points.push_back((int)(s * (x3 - cx) + c * (y3 - cy) + cy));
                obb_points.push_back((int)(c * (x4 - cx) - s * (y4 - cy) + cx));
                obb_points.push_back((int)(s * (x4 - cx) + c * (y4 - cy) + cy));
            }
            return obb_points;
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

        /**
         * keypoints
         * @maixpy maix.nn.Object.points
         */
        std::vector<int> points;

        /**
         * Rotate angle, -9999 means not set, value is a percentage, need to multiply 180 to get the real angle or multiply PI to get the radian.
         * @maixpy maix.nn.Object.angle
         */
        float angle;

        /**
         * segmentation mask, uint8 list type, shape is h * w but flattened to one dimension, value fron 0 to 255.
         * @attention For efficiency, it's a pointer in C++, use this carefully!
         * @maixpy maix.nn.Object.seg_mask
         */
        image::Image *seg_mask;

        /**
         * For temperary usage, not for MaixPy API
         */
        void *temp;
    };

    /**
     * Object for detect result
     * @maixpy maix.nn.ObjectFloat
     */
    class ObjectFloat
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
         * @maixpy maix.nn.ObjectFloat.__init__
         * @maixcdk maix.nn.ObjectFloat.ObjectFloat
         */
        ObjectFloat(float x = 0, float y = 0, float w = 0, float h = 0, float class_id = 0, float score = 0, std::vector<float> points = std::vector<float>(), float angle = -1)
            : x(x), y(y), w(w), h(h), class_id(class_id), score(score), points(points), angle(angle), temp(NULL)
        {
        }

        ~ObjectFloat()
        {
        }

        /**
         * Object info to string
         * @return Object info string
         * @maixpy maix.nn.ObjectFloat.__str__
         * @maixcdk maix.nn.ObjectFloat.to_str
         */
        std::string to_str()
        {
            if(angle >= 0)
                return "x: " + std::to_string(x) + ", y: " + std::to_string(y) + ", w: " + std::to_string(w) + ", h: " + std::to_string(h) + ", class_id: " + std::to_string(class_id) + ", score: " + std::to_string(score) + ", angle: " + std::to_string(angle);
            else
                return "x: " + std::to_string(x) + ", y: " + std::to_string(y) + ", w: " + std::to_string(w) + ", h: " + std::to_string(h) + ", class_id: " + std::to_string(class_id) + ", score: " + std::to_string(score);
        }

        /**
         * Object left top coordinate x
         * @maixpy maix.nn.ObjectFloat.x
         */
        float x;

        /**
         * Object left top coordinate y
         * @maixpy maix.nn.ObjectFloat.y
         */
        float y;

        /**
         * Object width
         * @maixpy maix.nn.ObjectFloat.w
         */
        float w;

        /**
         * Object height
         * @maixpy maix.nn.ObjectFloat.h
         */
        float h;

        /**
         * Object class id
         * @maixpy maix.nn.ObjectFloat.class_id
         */
        float class_id;

        /**
         * Object score
         * @maixpy maix.nn.ObjectFloat.score
         */
        float score;

        /**
         * keypoints
         * @maixpy maix.nn.ObjectFloat.points
         */
        std::vector<float> points;

        /**
         * Rotate angle
         * @maixpy maix.nn.ObjectFloat.angle
        */
        float angle;

        /**
         * For temperary usage, not for MaixPy API
         */
        void *temp;
    };

    /**
     * Objects Class for detect result
     * @maixpy maix.nn.Objects
     */
    class Objects
    {
    public:
        /**
         * Constructor of Objects class
         * @maixpy maix.nn.Objects.__init__
         * @maixcdk maix.nn.Objects.Objects
         */
        Objects()
        {
        }

        ~Objects()
        {
            for (Object *obj : objs)
            {
                if (obj->seg_mask)
                {
                    delete obj->seg_mask;
                    obj->seg_mask = NULL;
                }
                delete obj;
            }
        }

        /**
         * Add object to objects
         * @throw Throw exception if no memory
         * @maixpy maix.nn.Objects.add
         */
        nn::Object &add(int x = 0, int y = 0, int w = 0, int h = 0, int class_id = 0, float score = 0, std::vector<int> points = std::vector<int>(), float angle = -1)
        {
            Object *obj = new Object(x, y, w, h, class_id, score, points, angle);
            if(!obj)
                throw err::Exception(err::ERR_NO_MEM);
            obj->seg_mask = NULL;
            objs.push_back(obj);
            return *obj;
        }

        /**
         * Add object to objects
         * @throw Throw exception if no memory
         * @maixcdk maix.nn.Objects.add
         */
        nn::Object &add(const nn::Object &obj)
        {
            Object *obj_new = new Object(obj.x, obj.y, obj.w, obj.h, obj.class_id, obj.score, obj.points, obj.angle);
            if(!obj_new)
                throw err::Exception(err::ERR_NO_MEM);
            obj_new->seg_mask = NULL;
            objs.push_back(obj_new);
            return *obj_new;
        }

        /**
         * Remove object form objects
         * @maixpy maix.nn.Objects.remove
         */
        err::Err remove(int idx)
        {
            if ((size_t)idx >= objs.size())
                return err::ERR_ARGS;
            objs.erase(objs.begin() + idx);
            return err::ERR_NONE;
        }

        /**
         * Get object item
         * @maixpy maix.nn.Objects.at
         */
        nn::Object &at(int idx)
        {
            return *objs.at(idx);
        }

        /**
         * Get object item
         * @maixpy maix.nn.Objects.__getitem__
         * @maixcdk maix.nn.Objects.operator[]
         */
        nn::Object &operator[](int idx)
        {
            return *objs.at(idx);
        }

        /**
         * Get size
         * @maixpy maix.nn.Objects.__len__
         * @maixcdk maix.nn.Objects.size
         */
        size_t size()
        {
            return objs.size();
        }

        /**
         * Begin
          @maixpy maix.nn.Objects.__iter__
         * @maixcdk maix.nn.Objects.begin
        */
        std::vector<Object*>::iterator begin()
        {
            return objs.begin();
        }

        /**
         * End
         * @maixcdk maix.nn.Objects.end
        */
        std::vector<Object*>::iterator end()
        {
            return objs.end();
        }

    private:
        std::vector<Object *> objs;
    };
}
