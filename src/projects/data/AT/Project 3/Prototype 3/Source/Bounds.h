#pragma once
#include "Vec3.h"
#include "Triangles.h"
#include <string>

namespace MyMaths
{
	struct Bounds
	{
		Vec3f min, max;

		Bounds() :min(0), max(0) {};
		Bounds(const Vec3f& min, const Vec3f& max) :min(min), max(max) {};

		void setTo (Vec3f min, Vec3f max)
		{
			this->min = min; this->max = max;
		}

		/// <summary>
		/// Move the Bounds by <c>displacement</c>
		/// </summary>
		/// <returns>The Displaced Bounds</returns>
		Bounds operator+ (const Vec3f& displacement) const { return { this->min + displacement, this->max + displacement }; }
		Bounds operator+= (const Vec3f& displacement) { this->min += displacement; this->max += displacement; return *this; }

		std::string ToString()
		{
			return "[(" + std::to_string(min.x) + " " + std::to_string(min.y) + " " + std::to_string(min.z) + ") (" + std::to_string(max.x) + " " + std::to_string(max.y) + " " + std::to_string(max.z) + ")]";

		//	return os;
		}
		/// <returns>The Union of both Bounds</returns>
		static Bounds Union(const Bounds& left, const Bounds& right);
		/// <returns>The Union of a Bounds and a Triangle</returns>
		static Bounds Union(const Bounds& bounds, Vec3f& point);
		/// <returns>The Union of a Bounds and a Triangle</returns>
		static Bounds Union(const Bounds& bounds, Triangle& triangle);
		Bounds Union(Bounds& other);
		Bounds Union(Vec3f& point);
		Bounds Union(Triangle& triangle);
		/// <returns>If Two Bounds Intersect</returns>
		static const bool Intersect(const Bounds& left, const Bounds& right);
		static const bool Contains(const Bounds& bound, const Bounds& inside);
		bool Intersect(const Bounds& other);

		float distanceToSqrd(Vec3f& point);
		float distanceToSqrd(Triangle& triangle);
		/// <summary>
		/// Computes the Distance from this Bounds to another Bounds
		/// </summary>
		/// <param name="bounds"></param>
		/// <returns></returns>
		float distanceToSqrd(const Bounds& bounds) const;

		float size2()
		{
			auto delta = max - min;
			return delta.length2();
		}


		bool intersect(const Vec3f& rayorig, const Vec3f& raydir, float& t0, float& t1) const;
	};



	inline Bounds Bounds::Union(const Bounds& left, const Bounds& right)
	{
		Bounds newBounds;

		newBounds.min = std::min(left.min, right.min);
		newBounds.max = std::max(left.max, right.max);

		return newBounds;
	}
	inline Bounds Bounds::Union(const Bounds& bounds, Vec3f& point)
	{
		Bounds newBounds;

		newBounds.min = std::min(bounds.min, point);
		newBounds.max = std::max(bounds.max, point);

		return newBounds;
	}
	inline Bounds Bounds::Union(const Bounds& bounds, Triangle& triangle)
	{
		Bounds newBounds;

		newBounds.min = std::min(bounds.min, triangle.x);
		newBounds.max = std::max(bounds.max, triangle.x);

		newBounds.min = std::min(bounds.min, triangle.y);
		newBounds.max = std::max(bounds.max, triangle.y);

		newBounds.min = std::min(bounds.min, triangle.z);
		newBounds.max = std::max(bounds.max, triangle.z);

		return newBounds;
	}

	inline Bounds Bounds::Union(Bounds& other)
	{
		*this = Bounds::Union(*this, other);
		return *this;
	}
	inline Bounds Bounds::Union(Vec3f& point)
	{
		*this = Bounds::Union(*this, point);
		return *this;
	}
	inline Bounds Bounds::Union(Triangle& triangle)
	{
		*this = Bounds::Union(*this, triangle);
		return *this;
	}

	inline const bool Bounds::Intersect(const Bounds& left, const Bounds& right)
	{
		Vec3f mid;
		mid = left.max;
		bool leftMaxIntersect = (right.min <= mid && mid <= right.max);
		mid = left.min;
		bool leftMinIntersect = (right.min <= mid && mid <= right.max);

		return leftMinIntersect || leftMaxIntersect;
	}
	inline bool Bounds::Intersect(const Bounds& other)
	{
		Vec3f mid;
		mid = this->max;
		bool leftMaxIntersect = (other.min <= mid && mid <= other.max);
		mid = this->min;
		bool leftMinIntersect = (other.min <= mid && mid <= other.max);

		return leftMinIntersect || leftMaxIntersect;
	}
		
	

	inline const bool Bounds::Contains(const Bounds& bound, const Bounds& inside)
	{
		bool minInX, maxInX,
			minInY, maxInY,
			minInZ, maxInZ = false;

		minInX = bound.min.x < inside.min.x&& inside.min.x < bound.max.x;
		maxInX = bound.min.x < inside.max.x&& inside.max.x < bound.max.x;
		minInY = bound.min.y < inside.min.y&& inside.min.y < bound.max.y;
		maxInY = bound.min.y < inside.max.y&& inside.max.y < bound.max.y;
		minInZ = bound.min.z < inside.min.z&& inside.min.z < bound.max.z;
		maxInZ = bound.min.z < inside.max.z&& inside.max.z < bound.max.z;

		return (minInX && minInY && minInZ &&
			maxInX && maxInY && maxInZ);
	}
	inline float Bounds::distanceToSqrd(Vec3f& point)
	{
		auto top = min.x; auto bot = max.x;
		auto left = min.y; auto right = max.y;
		auto front = min.z; auto back = max.z;

		/// The 6 Vertices of the Bounds Cuboid
		Vec3f topLeftFront = { min };
		Vec3f topRightFront = { topLeftFront }; topRightFront.y = left;
		Vec3f topRightBack = { topRightFront }; topRightBack.z = back;

		Vec3f botRightBack = { max };
		Vec3f botRightFront = botRightBack; botRightFront.z = front;
		Vec3f botLeftFront = botRightFront; botLeftFront.y = left;
		///

		(min - point).length2();
		(max - point).length2();
		return 0.0f;
	}
	inline float Bounds::distanceToSqrd(const Bounds& bounds) const
	{
		bool minInX, maxInX,
			minInY, maxInY,
			minInZ, maxInZ = false;
		// If min inside x bounds
		minInX = this->min.x < bounds.min.x&& bounds.min.x < this->max.x;
		// If max inside x bounds
		maxInX = this->min.x < bounds.max.x&& bounds.max.x < this->max.x;
		// If min inside y bounds
		minInY = this->min.y < bounds.min.y&& bounds.min.y < this->max.y;
		// If max inside y bounds
		maxInY = this->min.y < bounds.max.y&& bounds.max.y < this->max.y;
		// If min inside z bounds
		minInZ = this->min.z < bounds.min.z&& bounds.min.z < this->max.z;
		// If max inside z bounds
		maxInZ = this->min.z < bounds.max.z&& bounds.max.z < this->max.z;

		// If inside, distance is 0
		if (minInX && minInY && minInZ &&
			maxInX && maxInY && maxInZ)
		{
			return 0.f;
		}

		float min_dist = F_Infinity;
		if (!minInX)
		{
			min_dist = std::min(std::abs(bounds.min.x - this->min.x), min_dist);
		}
		if (!minInY)
		{
			min_dist = std::min(std::abs(bounds.min.y - this->min.y), min_dist);
		}
		if (!minInZ)
		{
			min_dist = std::min(std::abs(bounds.min.z - this->min.z), min_dist);
		}
		if (!maxInX)
		{
			min_dist = std::min(std::abs(bounds.max.x - this->max.x), min_dist);
		}
		if (!maxInY)
		{
			min_dist = std::min(std::abs(bounds.max.y - this->max.y), min_dist);
		}
		if (!maxInZ)
		{
			min_dist = std::min(std::abs(bounds.max.z - this->max.z), min_dist);
		}
		return min_dist;
	}
	inline bool Bounds::intersect(const Vec3f& rayorig, const Vec3f& raydir, float& t0, float& t1) const
	{
		// X Intersection
		float tmin = (min.x - rayorig.x) / raydir.x;
		float tmax = (max.x - rayorig.x) / raydir.x;

		if (tmin > tmax) std::swap(tmin, tmax);

		// Y Intersection
		float tymin = (min.y - rayorig.y) / raydir.y;
		float tymax = (max.y - rayorig.y) / raydir.y;

		if (tymin > tymax) std::swap(tymin, tymax);

		if ((tmin > tymax) || (tymin > tmax))
			return false;

		if (tymin > tmin)
			tmin = tymin;

		if (tymax < tmax)
			tmax = tymax;

		// Z Intersection
		float tzmin = (min.z - rayorig.z) / raydir.z;
		float tzmax = (max.z - rayorig.z) / raydir.z;

		if (tzmin > tzmax) std::swap(tzmin, tzmax);

		if ((tmin > tzmax) || (tzmin > tmax))
			return false;

		if (tzmin > tmin)
			tmin = tzmin;

		if (tzmax < tmax)
			tmax = tzmax;

		t0 = tmin;
		t1 = tmax;
		return true;
	}
}