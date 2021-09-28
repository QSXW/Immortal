#pragma once

#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <iostream>

namespace Immortal
{
namespace Vector
{
using Vector2    = glm::vec2;
using Vector3    = glm::vec3;
using Vector4    = glm::vec4;
using Color      = glm::vec4;
using mat3       = glm::mat3;
using mat4       = glm::mat4;
using Matrix3    = mat3;
using Matrix4    = mat4;
using Quaternion = glm::quat;

static constexpr float PI = 3.14159f;

template <class T>
constexpr inline auto Normalize(T &v)
{
    return glm::normalize(v);
}

template <class T>
constexpr inline auto Cross(T &v1, T &v2)
{
    return glm::cross(v1, v2);
}

inline auto Radians(float degrees)
{
    return glm::radians(degrees);
}

inline auto Radians(Vector3 degrees)
{
    return glm::radians(degrees);
}

inline auto Degrees(float radians)
{
    return glm::degrees(radians);
}

inline auto Degrees(Vector3 radians)
{
    return glm::degrees(radians);
}

inline auto Translate(Vector3 position)
{
    return glm::translate(mat4(1.0f), position);
}

inline auto Rotate(float rotation, Vector3 direction)
{
    return glm::rotate(mat4(1.0f), glm::radians(rotation), direction);
}

/* Rotate a vector by a quaternion  */
template <class T>
inline auto Rotate(Quaternion &q, T v)
{
    return glm::rotate(q, v);
}

inline auto ToMatrix4(Quaternion &q)
{
    return glm::toMat4(q);
}

inline auto Rotate(Vector3 rotation)
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
inline auto Distance(T &p0, T &p1)
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

bool DecomposeTransform(const mat4& transform, Vector3& position, Vector3& rotation, Vector3& scale);
}

using Vector2    = Vector::Vector2;
using Vector3    = Vector::Vector3;
using Vector4    = Vector::Vector4;
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
