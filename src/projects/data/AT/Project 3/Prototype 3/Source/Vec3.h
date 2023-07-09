#pragma once

namespace MyMaths
{
    template<typename T>
    class Vec3
    {
    public:
        T x, y, z;
        Vec3() : x(T(0)), y(T(0)), z(T(0)) {}
        Vec3(T xx) : x(xx), y(xx), z(xx) {}
        Vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}
        // Copy by Value
        Vec3(Vec3<T>& vec) : x(vec.x), y(vec.y), z(vec.z) {}
        Vec3(const Vec3<T>& vec) : x(vec.x), y(vec.y), z(vec.z) {}

        Vec3& normalize()
        {
            T nor2 = length2();
            if (nor2 > 0) {
                T invNor = 1 / sqrt(nor2);
                x *= invNor, y *= invNor, z *= invNor;
            }
            return *this;
        }
        Vec3<T> operator * (const T& f) const { return Vec3<T>(x * f, y * f, z * f); }
        Vec3<T> operator * (const Vec3<T>& v) const { return Vec3<T>(x * v.x, y * v.y, z * v.z); }
        Vec3<T> operator / (const T& f) const { return Vec3<T>(x / f, y / f, z / f); }
        Vec3<T> operator / (const Vec3<T>& v) const { return Vec3<T>(x / v.x, y / v.y, z / v.z); }
        T dot(const Vec3<T>& v) const { return x * v.x + y * v.y + z * v.z; }
        Vec3<T> operator - (const Vec3<T>& v) const { return Vec3<T>(x - v.x, y - v.y, z - v.z); }
        Vec3<T> operator + (const Vec3<T>& v) const { return Vec3<T>(x + v.x, y + v.y, z + v.z); }
        Vec3<T>& operator += (const Vec3<T>& v) { x += v.x, y += v.y, z += v.z; return *this; }
        Vec3<T>& operator *= (const Vec3<T>& v) { x *= v.x, y *= v.y, z *= v.z; return *this; }

        bool operator == (const Vec3<T>& v) const { return (this->x == v.x && this->y == v.y && this->z == v.z); }
        bool operator < (const Vec3<T>& v) const { return (this->x < v.x&& this->y < v.y&& this->z < v.z); }
        bool operator > (const Vec3<T>& v) const { return v < *this; }

        bool operator <= (const Vec3<T>& v) const { return ((*this < v) || (*this == v)); }
        bool operator >= (const Vec3<T>& v) const { return ((*this > v) || (*this == v)); }

        bool operator == (const T& v) const { return (this->x == v && this->y == v && this->z == v); }
        bool operator < (const T& v) const { return (this->x < v && this->y < v && this->z < v); }
        bool operator > (const T& v) const { return v < *this; }

        bool operator <= (const T& v) const { return ((*this < v) || (*this == v)); }
        bool operator >= (const T& v) const { return ((*this > v) || (*this == v)); }

        operator bool() const { return x != 0 && y != 0 && z != 0; }

        T& operator[](const size_t index) const
        {
            switch (index)
            {
            case(0): { return x; }
            case(1): { return y; }
            case(2): { return z; }
            default: { return 0; }
            }
        }

        Vec3<T> operator - () const { return Vec3<T>(-x, -y, -z); }
        T length2() const { return x * x + y * y + z * z; }
        T length() const { return sqrt(length2()); }
        //friend std::ostream& operator << (std::ostream& os, const Vec3<T>& v)
        //{
        //    os << "[" << v.x << " " << v.y << " " << v.z << "]";
        //    return os;
        //}
    };

    // Perpendicular to Both Vectors
    template<typename Type>
    Vec3<Type> cross(const Vec3<Type>& vec_a, const Vec3<Type>& vec_b)
    {
        return Vec3<Type>(
            vec_a.y * vec_b.z - vec_a.z * vec_b.y,
            vec_a.z * vec_b.x - vec_a.x * vec_b.z,
            vec_a.x * vec_b.y - vec_a.y * vec_b.x
            );
    };

    // O = A • B
    // a = x * x
    // b = y * y
    // c = z * z
    template<typename Type>
    Type dot(const Vec3<Type>& vec_a, const Vec3<Type>& vec_b)
    {
        return vec_a.x * vec_b.x + vec_a.y * vec_b.y + vec_a.z * vec_b.z;
    };


    typedef Vec3<float> Vec3f;
}