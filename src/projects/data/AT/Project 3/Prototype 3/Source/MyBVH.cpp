#include "MyBVH.h"



void Branch::addFragment(Fragment& fragment)
{
	// Get Relevant Bounds
	auto newBounds = fragment.bounds;
	auto rightBounds = right->getBounds();
	auto leftBounds = left->getBounds();
	// Find where that new Bounds exists in this Branch
	auto inRight = rightBounds.Intersect(newBounds);
	auto inLeft = leftBounds.Intersect(newBounds);
	auto inBoth = inRight && inLeft;
	auto inNeither = !(inRight || inLeft);
	// This is a Problem
	if (inBoth)
	{
		std::cout << "Uh oh" << std::endl;
		return;
	}
	// The Most Common, Probably
	if (inNeither)
	{
		// Find the Closest Bounds
		auto rightDist = rightBounds.distanceToSqrd(newBounds);
		auto leftDist = leftBounds.distanceToSqrd(newBounds);
		if (rightDist > leftDist)
		{
			left->addFragment(fragment);
		}
		else
		{
			right->addFragment(fragment);
		}

	}
	else
	{
		if (inLeft)
		{
			left->addFragment(fragment);
		}
		if (inRight)
		{
			right->addFragment(fragment);
		}
	}

	this->bounds = Bounds::Union(this->bounds, fragment.bounds);
	fragmentCount++;
}
void Leaf::addFragment(Fragment& fragment)
{
	this->fragments.push_back(fragment);
	this->bounds = Bounds::Union(this->bounds, fragment.bounds);
	fragmentCount++;

	recalculateBounds();
}
void TreeThing::addFragment(Fragment& fragment)
{
	std::visit([&](auto& visitedThing) {return visitedThing->addFragment(fragment); }, thing);
	fragmentCount++;
}


bool Branch::intersect(const Vec3f& rayorig, const Vec3f& raydir, float& tnear, const Shape*& hitShape, bool ignoreShape) const
{
	float discard;
	if (right->getBounds().intersect(rayorig, raydir, discard, discard))
	{
		// If return is because my BSP cutter is bad
		if (right->intersect(rayorig, raydir, tnear, hitShape, ignoreShape)) return true;
	}
	if (left->getBounds().intersect(rayorig, raydir, discard, discard))
	{
		if (left->intersect(rayorig, raydir, tnear, hitShape, ignoreShape)) return true;
	}
	return false;
}
bool Leaf::intersect(const Vec3f& rayorig, const Vec3f& raydir, float& tnear, const Shape*& hitShape, bool ignoreShape) const
{
	tnear = INFINITY;

	// find closest shape this ray passes through
	for (unsigned i = 0; i < fragments.size(); ++i) {
		if (ignoreShape && hitShape == fragments[i].shape) { continue; }
		float t0 = INFINITY, t1 = INFINITY;
		if (fragments[i].intersect(rayorig, raydir, t0, t1)) {
			//printf("Hit Shape\n");
			if (t0 < 0) t0 = t1;
			if (t0 < tnear) {
				tnear = t0;
				hitShape = fragments[i].shape;
			}
		}
	}
	return (hitShape);
}
bool TreeThing::intersect(const Vec3f& rayorig, const Vec3f& raydir, float& tnear, const Shape*& hitShape, bool ignoreShape) const
{
	return std::visit([&](auto& visitedThing) {return visitedThing->intersect(rayorig, raydir, tnear, hitShape, ignoreShape); }, thing);
}

void Leaf::recalculateBounds()
{
	Vec3f min{ MyMaths::F_Infinity }, max{ 0.f };
	for (auto& fragment : fragments)
	{
		min = std::min(fragment.bounds.min, min);
		max = std::max(fragment.bounds.max, max);
	}
}

Branch TreeThing::split()
{
	if (isLeaf)
	{
		// Split it time :)
		auto leaf = std::get<Leaf*>(thing);
		auto midpoint = (leaf->bounds.min + leaf->bounds.max) / 2.f;
		auto fragments = leaf->fragments;


		MyMaths::Bounds newBoundsA{ leaf->bounds.min, midpoint };
		std::vector<Fragment> bucketA, bucketB;
		for (auto& fragment : fragments)
		{
			if (newBoundsA.Intersect(fragment.bounds))
			{
				bucketA.push_back(fragment);
			}
			else // We can assume it's inside B, because it had to be in the entire thing
			{
				bucketB.push_back(fragment);
			}
		}

		// Replace Leaf with new Branch
		thing = new Branch(
			new TreeThing(
				new Leaf(bucketA,
					{ leaf->bounds.min, midpoint })),
			new TreeThing(
				new Leaf(bucketB,
					{ midpoint, leaf->bounds.max }))
		);
		isLeaf = false;
	}
	else
	{
		// Let the Branches Split themselves
		auto branch = std::get<Branch*>(thing);


		branch->right->split();
		branch->left->split();
	}

	return *std::get<Branch*>(thing);
}

Bounds& TreeThing::getBounds() const
{
	// Go to the Thing inside of TheThing and get the bound's pointer, and then fetch the value of that
	if (isLeaf)
	{
		return std::get<Leaf*>(thing)->bounds;
	}
	else
	{
		return std::get<Branch*>(thing)->bounds;
	}
	//return *std::visit([](auto& visitedThing) {return &visitedThing->bounds; }, thing);
}


std::vector<const Light*> Branch::getLights() const
{
	auto Lights = left->getLights();
	auto RightSide = right->getLights();
	Lights.reserve(Lights.size() + RightSide.size());
	Lights.insert(Lights.end(), std::make_move_iterator(RightSide.begin()), std::make_move_iterator(RightSide.end()));
	return Lights;
}
std::vector<const Light*> Leaf::getLights()
{
	auto out = std::vector<const Light*> ();
	for (const auto& fragment : fragments)
	{
		if (fragment.shape->emissionColour > 0.f)
		{
			auto light = new Light();
			light->emissionColour = fragment.shape->emissionColour;
			light->position = fragment.shape->position;
			out.push_back(light);
		}
	}

	return out;
}
std::vector<const Light*> TreeThing::getLights() const
{
	return std::visit([&](auto& visitedThing) {return visitedThing->getLights(); }, thing);
}
