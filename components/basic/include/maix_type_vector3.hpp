#pragma once

#include "math.h"

namespace maix::tensor
{
    template <typename T>
    class Vector3
    {
    public:
        T x,y,z;
    public:
        //defualt constructor
        Vector3() { x = 0;  y = 0; z = 0; }
        // setting ctor
        Vector3(T x0, T y0, T z0) : x(x0), y(y0), z(z0) {}
        //"()" overload 
        void operator()(T x0, T y0, T z0){x= x0; y= y0; z= z0;}
        //"==" overload
        bool operator==(const Vector3<T> &v){return (x==v.x && y==v.y && z==v.z); }
        //"!=" overload
        bool operator!=(const Vector3<T> &v){return (x!=v.x || y!=v.y || z!=v.z); }
        // "-" negation overload
        Vector3<T> operator-(void) const { return Vector3<T>(-x,-y,-z);}
        //"+" addition overload
        Vector3<T> operator+(const Vector3<T> &v) const { return Vector3<T>(x+v.x, y+v.y, z+v.z); }
        //"-" subtraction overload
        Vector3<T> operator-(const Vector3<T> &v) const { return Vector3<T>(x-v.x, y-v.y, z-v.z); }
        //"*" multiply overload
        Vector3<T> operator*(const T n)const { return Vector3<T>(x*n, y*n, z*n); }
        //"/" divsion overload
        Vector3<T> operator/(const T n)const { return Vector3<T>(x/n, y/n, z/n); }
        //"=" 
        Vector3<T> &operator=(const Vector3<T> &v){x=v.x; y=v.y; z=v.z; return *this;}
        //"+=" overload
        Vector3<T> &operator+=(const Vector3<T> &v) { x+=v.x; y+=v.y; z+=v.z; return *this; }
        //"-=" overload
        Vector3<T> &operator-=(const Vector3<T> &v) { x-=v.x; y-=v.y; z-=v.z; return *this; }
        //"*=" overload
        Vector3<T> &operator*=(const T n) {  x*=n; y*=n; z*=n;  return *this; }
        // uniform scaling
        Vector3<T> &operator/=(const T n) {  x/=n; y/=n; z/=n;  return *this; }
        //dot product
        T operator*(const Vector3<T> &v) const { return x*v.x + y*v.y + z*v.z;  }
        //cross product
        Vector3<T> operator %(const Vector3<T> &v)const { return Vector3<T>(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x);}
        // gets the length of this vector squared
        T LengthSquared() const  {   return (T)(*this * *this); }
        // gets the length of this vector
        float Length(void) const  {   return (T)sqrt(*this * *this); }
        // normalizes this vector
        void Normalize() {  *this/=Length(); }
        // zero the vector
        void Zero()   {  x = y = z = 0.0;  }
        // returns the normalized version of this vector
        Vector3<T>  Normalized() const  {   return *this/Length();   }
        // check if any elements are NAN
        bool IsNan(void) { return isnan(x) || isnan(y) || isnan(z); }
        // check if any elements are infinity
        bool IsInf(void) { return isinf(x) || isinf(y) || isinf(z); }

    };

    /**
    * Vector3 float type.
    * @maixpy maix.tensor.Vector3f
    */
    class Vector3f : public Vector3<float> // we use class for we want to generate maixpy API, and the tool not suppoort using yet.
    {
    public:
        /**
          * default constructor
          * @maixcdk maix.tensor.Vector3f.Vector3f
         */
        Vector3f() : Vector3<float>() {}

        /**
          * Construct Vector3f with 3 variables.
          * @maixpy maix.tensor.Vector3f.__init__
          * @maixcdk maix.tensor.Vector3f.Vector3f
         */
        Vector3f(float x0, float y0, float z0)
        : Vector3<float>(x0, y0, z0)
        {}

        /**
          * member x.
          * @maixpy maix.tensor.Vector3f.x
         */
        // float x;

        /**
          * member y.
          * @maixpy maix.tensor.Vector3f.y
         */
        // float y;

        /**
          * member z.
          * @maixpy maix.tensor.Vector3f.z
         */
        // float z;
    };

    /**
      * Vector3 int32_t type.
      * @maixpy maix.tensor.Vector3i32
     */
    class Vector3i32 : public Vector3<int32_t> // we use class for we want to generate maixpy API, and the tool not suppoort using yet.
    {
    public:
        /**
          * default constructor
          * @maixcdk maix.tensor.Vector3i32.Vector3i32
         */
         Vector3i32() : Vector3<int32_t>() {}

        /**
          * Construct Vector3i32 with 3 variables.
          * @maixpy maix.tensor.Vector3i32.__init__
          * @maixcdk maix.tensor.Vector3i32.Vector3i32
         */
         Vector3i32(int32_t x0, int32_t y0, int32_t z0)
        : Vector3<int32_t>(x0, y0, z0)
        {}

        /**
          * member x.
          * @maixpy maix.tensor.Vector3i32.x
         */
        // int32_t x;

        /**
          * member y.
          * @maixpy maix.tensor.Vector3i32.y
         */
        // int32_t y;

        /**
          * member z.
          * @maixpy maix.tensor.Vector3i32.z
         */
        // int32_t z;
    };

    /**
      * Vector3 uint32_t type.
      * @maixpy maix.tensor.Vector3u32
     */
     class Vector3u32 : public Vector3<uint32_t> // we use class for we want to generate maixpy API, and the tool not suppoort using yet.
     {
     public:
         /**
           * default constructor
           * @maixcdk maix.tensor.Vector3u32.Vector3u32
          */
          Vector3u32() : Vector3<uint32_t>() {}

         /**
           * Construct Vector3u32 with 3 variables.
           * @maixpy maix.tensor.Vector3u32.__init__
           * @maixcdk maix.tensor.Vector3u32.Vector3u32
          */
          Vector3u32(uint32_t x0, uint32_t y0, uint32_t z0)
         : Vector3<uint32_t>(x0, y0, z0)
         {}

        /**
          * member x.
          * @maixpy maix.tensor.Vector3u32.x
         */
        // uint32_t x;

        /**
          * member y.
          * @maixpy maix.tensor.Vector3u32.y
         */
        // uint32_t y;

        /**
          * member z.
          * @maixpy maix.tensor.Vector3u32.z
         */
        // uint32_t z;
     };

    /**
      * Vector3 int16 type.
      * @maixpy maix.tensor.Vector3i16
     */
     class Vector3i16 : public Vector3<int16_t> // we use class for we want to generate maixpy API, and the tool not suppoort using yet.
     {
     public:
         /**
           * default constructor
           * @maixcdk maix.tensor.Vector3i16.Vector3i16
          */
          Vector3i16() : Vector3<int16_t>() {}

         /**
           * Construct Vector3i16 with 3 variables.
           * @maixpy maix.tensor.Vector3i16.__init__
           * @maixcdk maix.tensor.Vector3i16.Vector3i16
          */
          Vector3i16(int16_t x0, int16_t y0, int16_t z0)
         : Vector3<int16_t>(x0, y0, z0)
         {}

        /**
          * member x.
          * @maixpy maix.tensor.Vector3i16.x
         */
        // int16_t x;

        /**
          * member y.
          * @maixpy maix.tensor.Vector3i16.y
         */
        // int16_t y;

        /**
          * member z.
          * @maixpy maix.tensor.Vector3i16.z
         */
        // int16_t z;
     };

    /**
      * Vector3 uint16 type.
      * @maixpy maix.tensor.Vector3u16
     */
     class Vector3u16 : public Vector3<uint16_t> // we use class for we want to generate maixpy API, and the tool not suppoort using yet.
     {
     public:
         /**
           * default constructor
           * @maixcdk maix.tensor.Vector3u16.Vector3u16
          */
          Vector3u16() : Vector3<uint16_t>() {}

         /**
           * Construct Vector3u16 with 3 variables.
           * @maixpy maix.tensor.Vector3u16.__init__
           * @maixcdk maix.tensor.Vector3u16.Vector3u16
          */
          Vector3u16(uint16_t x0, uint16_t y0, uint16_t z0)
         : Vector3<uint16_t>(x0, y0, z0)
         {}

        /**
          * member x.
          * @maixpy maix.tensor.Vector3u16.x
         */
        // uint16_t x;

        /**
          * member y.
          * @maixpy maix.tensor.Vector3u16.y
         */
        // uint16_t y;

        /**
          * member z.
          * @maixpy maix.tensor.Vector3u16.z
         */
        // uint16_t z;
     };

} //namespace maix


