/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2024.9.24: Create this file.
 */

#pragma once

#include <vector>
#include <string>


namespace maix::nn
{
    /**
     * Object for OCR detect box
     * @maixpy maix.nn.OCR_Box
     */
    class OCR_Box
    {
    public:
        /**
         * OCR_Box constructor
         * @maixpy maix.nn.OCR_Box.__init__
         * @maixcdk maix.nn.OCR_Box.OCR_Box
        */
        OCR_Box(int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0, int x3 = 0, int y3 = 0, int x4 = 0, int y4 = 0)
            :x1(x1), y1(y1), x2(x2), y2(y2), x3(x3), y3(y3), x4(x4), y4(y4)
        {

        }

        /**
         * left top point of box
         * @maixpy maix.nn.OCR_Box.x1
        */
        int x1;

        /**
         * left top point of box
         * @maixpy maix.nn.OCR_Box.y1
        */
        int y1;

        /**
         * right top point of box
         * @maixpy maix.nn.OCR_Box.x2
        */
        int x2;

        /**
         * right top point of box
         * @maixpy maix.nn.OCR_Box.y2
        */
        int y2;

        /**
         * right bottom point of box
         * @maixpy maix.nn.OCR_Box.x3
        */
        int x3;

        /**
         * right bottom point of box
         * @maixpy maix.nn.OCR_Box.y3
        */
        int y3;

        /**
         * left bottom point of box
         * @maixpy maix.nn.OCR_Box.x4
        */
        int x4;

        /**
         * left bottom point of box
         * @maixpy maix.nn.OCR_Box.y4
        */
        int y4;

        /**
         * convert box point to a list type.
         * @return list type, element is int type, value [x1, y1, x2, y2, x3, y3, x4, y4].
         * @maixpy maix.nn.OCR_Box.to_list
        */
        std::vector<int> to_list()
        {
            std::vector<int> li(8);
            li.at(0) = x1;
            li.at(1) = y1;
            li.at(2) = x2;
            li.at(3) = y2;
            li.at(4) = x3;
            li.at(5) = y3;
            li.at(6) = x4;
            li.at(7) = y4;
            return li;
        }
    };

    /**
     * Object for OCR detect result
     * @maixpy maix.nn.OCR_Object
     */
    class OCR_Object
    {
    public:
        /**
         * Constructor of Object for OCR detect result
         * @param score score
         * @maixpy maix.nn.OCR_Object.__init__
         * @maixcdk maix.nn.OCR_Object.OCR_Object
         */
        OCR_Object(const nn::OCR_Box &box, const std::vector<int> &idx_list, const std::vector<std::string> &char_list, float score = 0, const std::vector<int> &char_pos = std::vector<int>())
        :box(box), score(score), idx_list(idx_list), char_pos(char_pos), _char_list(char_list)
        {
            _chars.clear();
            for(const auto &c : _char_list)
            {
                _chars += c;
            }
        }

        ~OCR_Object()
        {
        }

        /**
         * OCR_Object box, 4 points box, first point at the left-top, clock-wise.
         * @maixpy maix.nn.OCR_Object.box
         */
        nn::OCR_Box box;

        /**
         * Object score
         * @maixpy maix.nn.OCR_Object.score
         */
        float score;

        /**
         * chars' idx list, element is int type.
         * @maixpy maix.nn.OCR_Object.idx_list
        */
        std::vector<int> idx_list;

        /**
         * Chars' position relative to left
         * @maixpy maix.nn.OCR_Object.char_pos
        */
        std::vector<int> char_pos;

        /**
         * Get OCR_Object's charactors, return a string type.
         * @return All charactors in string type.
         * @maixpy maix.nn.OCR_Object.char_str
        */
        const std::string &char_str()
        {
            return _chars;
        }

        /**
         * Get OCR_Object's charactors, return a list type.
         * @return All charactors in list type.
         * @maixpy maix.nn.OCR_Object.char_list
        */
        const std::vector<std::string> &char_list()
        {
            return _char_list;
        }

        /**
         * Set OCR_Object's charactors
         * @param char_list All charactors in list type.
         * @maixpy maix.nn.OCR_Object.update_chars
        */
        void update_chars(const std::vector<std::string> &char_list)
        {
            _char_list = char_list;
            _chars.clear();
            for(const auto &c : _char_list)
            {
                _chars += c;
            }
        }

        /**
         * OCR_Object info to string
         * @return OCR_Object info string
         * @maixpy maix.nn.OCR_Object.__str__
         * @maixcdk maix.nn.OCR_Object.to_str
         */
        std::string to_str()
        {
            return "left-top: (" + std::to_string(box.x1) + ", " + std::to_string(box.y1) + "), char num: " + std::to_string(_char_list.size()) + ", chars: " + _chars + ", score: " + std::to_string(score);
        }
    private:
        std::vector<std::string> _char_list;
        std::string _chars;
    };

    /**
     * OCR_Objects Class for detect result
     * @maixpy maix.nn.OCR_Objects
     */
    class OCR_Objects
    {
    public:
        /**
         * Constructor of OCR_Objects class
         * @maixpy maix.nn.OCR_Objects.__init__
         * @maixcdk maix.nn.OCR_Objects.OCR_Objects
         */
        OCR_Objects()
        {
        }

        ~OCR_Objects()
        {
            for (OCR_Object *obj : objs)
            {
                delete obj;
            }
        }

        /**
         * Add object to objects
         * @throw Throw exception if no memory
         * @maixpy maix.nn.OCR_Objects.add
         */
        nn::OCR_Object &add(const nn::OCR_Box &box, const std::vector<int> &idx_list, const std::vector<std::string> &char_list, float score = 0, const std::vector<int> &char_pos = std::vector<int>())
        {
            OCR_Object *obj = new OCR_Object(box, idx_list, char_list, score, char_pos);
            if(!obj)
                throw err::Exception(err::ERR_NO_MEM);
            objs.push_back(obj);
            return *obj;
        }

        /**
         * Remove object form objects
         * @maixpy maix.nn.OCR_Objects.remove
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
         * @maixpy maix.nn.OCR_Objects.at
         */
        nn::OCR_Object &at(int idx)
        {
            return *objs.at(idx);
        }

        /**
         * Get object item
         * @maixpy maix.nn.OCR_Objects.__getitem__
         * @maixcdk maix.nn.OCR_Objects.operator[]
         */
        nn::OCR_Object &operator[](int idx)
        {
            return *objs.at(idx);
        }

        /**
         * Get size
         * @maixpy maix.nn.OCR_Objects.__len__
         * @maixcdk maix.nn.OCR_Objects.size
         */
        size_t size()
        {
            return objs.size();
        }

        /**
         * Begin
          @maixpy maix.nn.OCR_Objects.__iter__
         * @maixcdk maix.nn.OCR_Objects.begin
        */
        std::vector<OCR_Object*>::iterator begin()
        {
            return objs.begin();
        }

        /**
         * End
         * @maixcdk maix.nn.OCR_Objects.end
        */
        std::vector<OCR_Object*>::iterator end()
        {
            return objs.end();
        }

    private:
        std::vector<OCR_Object *> objs;
    };
}
