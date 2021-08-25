#pragma once

#if 0
#include "ImmortalCore.h"
#include <intrin.h>

namespace Immortal
{
	union IMMORTAL_API Vector
	{
		Vector(float x, float y, float z = 0.0, float w = 1.0)
			: position{ x, y, z, w }
		{

		}

		__m128 reg;
		struct
		{
			float x;
			float y;
			float z;
			float w;
		} position;
	};
	 
	using Vector2 = Vector;
	using Vector3 = Vector;
	using Vector4 = Vector;
}
#endif


#ifndef __IMMORTAL_VECTOR_H__
#define __IMMORTAL_VECTOR_H__

#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

#define LOG_VEC2(v) LOG::INFO("{0} {1}", v.x, v.y)
#define LOG_VEC3(v) LOG::INFO("{0} {1} {2}", v.x, v.y, v.z)
#define LOG_VEC4(v) LOG::INFO("\n{0} {1} {2} {3}", v.x, v.y, v.z, v.w)
#define LOG_MAT4(v) LOG::INFO("\n{0} {1} {2} {3}\n{4} {5} {6} {7} \n{8} {9} {10} {11}\n{11} {12} {13} {14}\n", v[0].x, v[0].y, v[0].z, v[0].w, v[1].x, v[1].y, v[1].z, v[1].w, v[2].x, v[2].y, v[2].z, v[2].w, v[3].x, v[3].y, v[3].z, v[3].w)


namespace Immortal {

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

	using Vector2 = Vector::Vector2;
	using Vector3 = Vector::Vector3;
	using Vector4 = Vector::Vector4;
}
#endif /* __IMMORTAL_VECTOR_H__ */
