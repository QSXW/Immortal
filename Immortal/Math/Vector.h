#pragma once

#include <iostream>

#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace Immortal
{
namespace Vector
{

struct Vector2 : public glm::vec2
{
    using Primitive = glm::vec2;

    Vector2() :
        Primitive{}
    {}

    Vector2(float v) :
        Primitive{ v }
    {}

    template <class T, class U>
    Vector2(T x, U y) :
        Primitive{ (float)x, (float)y }
    {}

    Vector2(const Primitive &v) :
        Primitive{ v }
    {}

    float &operator[](size_t i)
    {
        return Primitive{ *this } [i] ;
    }

    Vector2 &operator+=(const Vector2 &v)
    {
        Primitive{ *this } += Primitive{ v };
        return *this;
    }

    float Length() const
    {
        return glm::length(Primitive{ *this });
    }

    Vector2 Degrees() const
    {
        return glm::degrees(Primitive{ *this });
    }

    Vector2 Radians() const
    {
        return glm::radians(Primitive{ *this });
    }

    float Distance(const Vector2 &distance) const
    {
        return glm::distance(Primitive{ *this }, distance);
    }

    Vector2 Normalize() const
    {
        return glm::normalize(*this);
    }
};

struct Vector4;
struct Vector3 : public glm::vec3
{
    using Primitive = glm::vec3;

    Vector3() :
        Primitive{}
    {}

    Vector3(float v) :
        Primitive{ v }
    {}

    template <class T, class U, class V>
    Vector3(T x, U y, V z) :
        Primitive{ (float)x, (float)y, (float)z }
    {}

    Vector3(const Primitive &v) :
        Primitive{ v }
    {}

    Vector3(const Vector4 &v);

    float &operator[](size_t i)
    {
        Primitive &p = *this;
        return p[i] ;
    }

    template <class T>
    Vector3 &operator+=(T v)
    {
        Primitive &p = *this;

        if constexpr (std::is_same_v<T, Vector3>)
        {
            p += Primitive{ v };
        }
        else
        {
            p += v;
        }
        return *this;
    }

    float Length() const
    {
        return glm::length(Primitive{ *this });
    }

    Vector3 Degrees() const
    {
        return glm::degrees(Primitive{ *this });
    }

    Vector3 Radians() const
    {
        return glm::radians(Primitive{ *this });
    }

    float Distance(const Vector3 &distance) const
    {
        return glm::distance(Primitive{ *this }, distance);
    }

    Vector3 Normalize() const
    {
        return glm::normalize(*this);
    }
};

struct Vector4 : public glm::vec4
{
    using Primitive = glm::vec4;

    Vector4() :
        Primitive{}
    {}

    Vector4(float v) :
        Primitive{ v }
    {}

    Vector4(float x, float y, float z, float w) :
        Primitive{ x, y, z, w }
    {}

    Vector4(const Vector3 &v, float w) :
        Primitive{ Vector3::Primitive{ v }, w }
    {

    }

    Vector4(const Primitive &v) :
        Primitive{ v }
    {}

    float &operator[](size_t i)
    {
        return ((Primitive&)*this)[i] ;
    }

    Vector4 &operator+=(const Vector4 &v)
    {
        Primitive{ *this } += Primitive{ v };
        return *this;
    }

    float Length() const
    {
        return glm::length(Primitive{ *this });
    }

    Vector4 Degrees() const
    {
        return glm::degrees(Primitive{ *this });
    }

    Vector4 Radians() const
    {
        return glm::radians(Primitive{ *this });
    }

    float Distance(const Vector4 &distance) const
    {
        return glm::distance(Primitive{ *this }, distance);
    }

    Vector4 Normalize() const
    {
        return glm::normalize(*this);
    }
};

inline Vector3::Vector3(const Vector4 &v) :
    Primitive{ Vector4::Primitive{ v } }
{

}

using Color      = glm::vec4;
using mat3       = glm::mat3;
using mat4       = glm::mat4;
using Matrix3    = mat3;
using Matrix4    = mat4;
using Quaternion = glm::quat;

template <class T>
constexpr inline auto Normalize(T v)
{
    return glm::normalize(v);
}

template <class T>
constexpr inline auto Cross(T v1, T v2)
{
    return glm::cross(v1, v2);
}

inline auto Radians(float degrees)
{
    return glm::radians(degrees);
}

inline auto Degrees(float radians)
{
    return glm::degrees(radians);
}

inline auto Translate(Vector3 position)
{
    return glm::translate(mat4(1.0f), position);
}

inline auto ToMatrix4(Quaternion &q)
{
    return glm::toMat4(q);
}

inline auto Rotate(float rotation, Vector3 direction)
{
    return glm::rotate(mat4(1.0f), glm::radians(rotation), direction);
}

/* Rotate a vector by a quaternion  */
template <class T>
inline auto Rotate(Quaternion q, T v)
{
    return glm::rotate(q, v);
}

inline glm::mat4 Rotate(Vector3 rotation)
{
    return glm::toMat4(Quaternion(rotation));
}

inline auto Scale(Vector3 scala)
{
    return glm::scale(mat4(1.0f), scala);
}

inline auto Inverse(mat4 matrix)
{
    return glm::inverse(matrix);
}

inline auto Ortho(float left, float right, float bottom, float top, float zNear, float zFar)
{
    return glm::ortho(left, right, bottom, top, zNear, zFar);
}

inline auto PerspectiveFOV(float fov, float width, float height, float zNear, float zFar)
{
    return glm::perspectiveFov(fov, width, height, zNear, zFar);
}

inline auto EpsilonEqual(const float &x, const float &y, const float &epsilon)
{
    return glm::epsilonEqual(x, y, epsilon);
}

inline auto EpsilonNotEqual(const float &x, const float &y, const float &epsilon)
{
    return glm::epsilonNotEqual(x, y, epsilon);
}

template <class T>
inline auto Distance(T p0, T p1)
{
    return glm::distance(p0, p1);
}

template <class T>
inline auto EulerAngles(T &x)
{
    return glm::eulerAngles(x);
}

template <class T>
inline auto Epsilon()
{
    return glm::epsilon<T>();
}

template <class T>
inline auto Length(T &q)
{
    return glm::length(q);
}

namespace Detail = glm::detail;

bool DecomposeTransform(const mat4 &transform, Vector3 &position, Vector3 &rotation, Vector3 &scale);

}

using Vector2    = Vector::Vector2;
using Vector3    = Vector::Vector3;
using Vector4    = Vector::Vector4;
using Matrix3    = Vector::Matrix3;
using Matrix4    = Vector::Matrix4;
using Color      = Vector::Color;
using Quaternion = Vector::Quaternion;

static inline std::ostream &operator <<(std::ostream &os, Vector2 &v)
{
    printf("%g, %g", v.x, v.y);
    return os;
}

static inline std::ostream &operator <<(std::ostream &os, Vector3 &v)
{
    printf("%g, %g, %g", v.x, v.y, v.z);
    return os;
}

static inline std::ostream &operator <<(std::ostream &os, Vector4 &v)
{
    printf("%g, %g, %g, %g", v.x, v.y, v.z, v.w);
    return os;
}

static inline std::ostream &operator <<(std::ostream &os, Matrix4 &m)
{
    for (int i = 0; i < 4; i++)
    {
        std::cout << *(Vector4 *)(&m) << std::endl;
    }

    return os;
}

}
