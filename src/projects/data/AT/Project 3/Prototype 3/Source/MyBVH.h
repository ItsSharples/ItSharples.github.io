#pragma once

#include "MyMaths.h"
#include <variant>


constexpr auto BOUNDS_LIMIT = 200.f;
using Vec3f = MyMaths::Vec3f;
using Bounds = MyMaths::Bounds;
using Shape = MyMaths::Shape;

struct Fragment;
struct Leaf;
struct Branch;
struct TreeThing;
typedef std::variant<Leaf*, Branch*> TheThing;

struct Light
{
	Vec3f emissionColour;
	Vec3f position;
};
struct Fragment
{
	Shape* shape;
	const Bounds bounds;

	Fragment(Shape* shape, const Bounds bounds) :shape(shape), bounds(bounds) {};
	Fragment(const Fragment& other) :shape(other.shape), bounds(other.bounds) {};
	bool intersect(const Vec3f& rayorig, const Vec3f& raydir, float& t0, float& t1) const
	{
		return shape->intersect(rayorig, raydir, t0, t1);
		/*
			printf("Hit\tColours: Surface: (%f, %f, %f)\tEmission: (%f, %f, %f)\n",
				shape->surfaceColour.x, shape->surfaceColour.y, shape->surfaceColour.z,
				shape->emissionColour.x, shape->emissionColour.y, shape->emissionColour.z);
		}*/
	}
};
struct Leaf
{
	Leaf() : fragments(), bounds() {};
	Leaf(std::vector<Fragment> fragment, Bounds bound) :fragments(fragment), bounds(bound) {};


	std::vector<Fragment> fragments;
	unsigned long fragmentCount = 0;
	Bounds bounds;

	void addFragment(Fragment& fragment);
	bool intersect(const Vec3f& rayorig, const Vec3f& raydir, float& tnear, const Shape*& hitShape, bool ignoreShape = false) const;
	std::vector<const Light*> getLights();
	void recalculateBounds();
};
struct TreeThing
{
public:
	
	TreeThing(Branch* newBranch)
	{
		thing = newBranch;
		//branch = newBranch;
		isLeaf = false;
		//bounds = newBranch->bounds;
	};
	TreeThing(Leaf* newLeaf)
	{
		thing = newLeaf;
		//leaf = newLeaf;
		isLeaf = true;
		//bounds = newLeaf->bounds;
	};

	Bounds& getBounds() const;
	std::vector<const Light*> getLights() const;
private:
	TheThing thing;
	//Leaf* leaf;
	//Branch* branch;
	Bounds bounds;
	bool isLeaf = false;
	unsigned long fragmentCount = 0;
public:
	void addFragment(Fragment& fragment);
	bool intersect(const Vec3f& rayorig, const Vec3f& raydir, float& tnear, const Shape*& hitShape, bool ignoreShape = false) const;

	Branch split();
};
struct Branch
{
	TreeThing* right = nullptr;
	TreeThing* left = nullptr;
	Bounds bounds;
	//std::shared_ptr<Bounds> rightBounds;
	//std::shared_ptr<Bounds> leftBounds;
	unsigned long fragmentCount = 0;

	bool intersect(const Vec3f& rayorig, const Vec3f& raydir, float& tnear, const Shape*& hitShape, bool ignoreShape = false) const;
	std::vector<const Light*> getLights() const;
	void addFragment(Fragment& fragment);

	Branch(TreeThing* left, TreeThing* right) :
		right(right),
		//rightBounds(&right->bounds),

		left(left),
		//leftBounds(&left->bounds),

		//bounds(Bounds::Union(right->getBounds(), left->getBounds()))
		bounds({ {-1000}, {1000}})
	{};
};
class MyBVH
{
	Branch parent;
	MyMaths::Bounds bounds;
public:
	MyBVH() : parent(new TreeThing(new Leaf()), new TreeThing(new Leaf())), bounds()
	{
		delete parent.left, parent.right;
		parent.left = nullptr;
		parent.right = nullptr;


		auto vecFrag = std::vector<Fragment>();
		parent.left = new TreeThing(new Leaf(vecFrag, {}));
		parent.right = new TreeThing(new Leaf(vecFrag, {}));

	}

	void addShape(Shape* newShape)
	{
		auto shapeFragment = Fragment(newShape, newShape->getBoundsWorldSpace());
		//parent.right->addFragment(shapeFragment);
		parent.addFragment(shapeFragment);
		bounds = Bounds::Union(bounds, newShape->getBoundsWorldSpace());
	}


	std::vector<const Light*> getLights() const
	{
		return parent.getLights();
	};

	Vec3f intersect(const Vec3f& rayorig, const Vec3f& raydir, int depth = 0) const
	{
		float tnear = INFINITY;
		const Shape* shape = NULL;
		// if there's no intersection return black or background color
		if (!parent.intersect(rayorig, raydir, tnear, shape)) return MyMaths::Colours::CornflowerBlue;

		//printf("Base Colour: Vec(%f,%f,%f)\n",shape->surfaceColour.x, shape->surfaceColour.y, shape->surfaceColour.z);
		//printf("%f, %f, %f\n", t0, t1, tnear);
		//return shape->surfaceColour;

		// Start Working out Colour and Such
		Vec3f surfaceColor = 0; // color of the ray/surface of the object intersected by the ray 
		Vec3f phit = rayorig + raydir * tnear; // point of intersection (origin + direction * distance)
		Vec3f nhit = phit - shape->position; // normal at the intersection point 
		nhit.normalize(); // normalize normal direction 

		// If the normal and the view direction are not opposite to each other reverse the normal direction. 
		// This also means we are inside the sphere

		float bias = 1e-4; // add some bias to the point from which we will be tracing 
		bool inside = false;
		if (raydir.dot(nhit) > 0) { nhit = -nhit, inside = true; }
		bool hasTransparency = shape->transparency > 0;
		if ((hasTransparency || shape->reflection > 0) && depth < MyMaths::MAX_RAY_DEPTH) {
			float facingratio = -raydir.dot(nhit);
			// change the mix value to tweak the effect
			float fresnelEffect = MyMaths::mix(pow(1.f - facingratio, 3), 1.f, 0.1f);
			// compute reflection direction (vectors are already normalized)
			// dir - 2 * (dir dot normal) *  normal is reflection
			Vec3f refldir = raydir + nhit * 2.f * facingratio;
			refldir.normalize();
			Vec3f reflection = intersect(phit + nhit * bias, refldir, depth + 1);
			Vec3f refraction = 0;
			// if the sphere is also transparent compute refraction ray (transmission)
			if (hasTransparency) {
				float ior = 1.1f, eta = (inside) ? ior : 1.f / ior; // are we inside or outside the surface? 
				float cosi = -nhit.dot(raydir);
				float k = 1.f - eta * eta * (1.f - cosi * cosi);
				Vec3f refractionDir = raydir * eta + nhit * (eta * cosi - sqrt(k));
				refractionDir.normalize();
				refraction = intersect(phit - nhit * bias, refractionDir, depth + 1);
			}
			// the result is a mix of reflection and refraction (if the sphere is transparent)
			surfaceColor = (
				reflection * fresnelEffect +
				refraction * (1.f - fresnelEffect) * shape->transparency) * shape->surfaceColour;
		}
		else {
			// it's a diffuse object, no need to raytrace any further
			for (auto& light : getLights())
			{
				Vec3f transmission = 1;
				Vec3f lightDirection = light->position - phit;
				lightDirection.normalize();
				if (parent.intersect(phit + nhit * bias, lightDirection, tnear, shape, true))
				{
					transmission = 0;
				}
				surfaceColor += shape->surfaceColour * transmission *
					std::max(float(0), nhit.dot(lightDirection)) * light->emissionColour;
			}
		}


		return surfaceColor + shape->emissionColour;
	}
};

