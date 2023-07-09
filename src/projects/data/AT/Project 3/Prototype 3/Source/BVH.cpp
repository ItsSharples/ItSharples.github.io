#include "BVH.h"


Node* BVH::buildNode(const NodeData& data, int level)
{
	constexpr size_t minLeafSize = 4; // The min number of triangles in a leaf
	constexpr size_t maxLeafSize = 400; // The max number of triangles in a leaf
	constexpr size_t MaxDepth = 20; // How deep can it go
	constexpr size_t triangleCost = 10;
	constexpr size_t nodeCost = 1;

	if (data.numChunks <= minLeafSize || level >= MaxDepth)
	{
		return createLeaf(data);
	}

	constexpr float m_minOverlap = 0.0f;

	// Find split candidates.

	float area = data.bounds.area();
	float leafSAH = area * triangleCost * data.numChunks;
	float nodeSAH = area * nodeCost * 2;
	ObjectSplit object = findObjectSplit(data, nodeSAH);

	SpatialSplit spatial;
	if (level < MaxSpatialDepth)
	{
		Bounds overlap = object.leftBounds;
		overlap.intersect(object.rightBounds);
		if (overlap.area() >= m_minOverlap)
			spatial = findSpatialSplit(data, nodeSAH);
	}

	// Leaf SAH is the lowest => create leaf.
	Vec3f MinSAH = std::min(leafSAH, object.cost, spatial.cost);
	float minSAH = std::min(MinSAH.x, MinSAH.y, MinSAH.z);
	if (minSAH == leafSAH && data.numChunks <= maxLeafSize) {
		return createLeaf(data);
	}

	// Leaf SAH is not the lowest => Perform spatial split.

	NodeData left, right;
	if (minSAH == spatial.cost) {
		performSpatialSplit(left, right, data, spatial);
	}

	if (!left.numChunks || !right.numChunks) { /// if either child contains no triangles/references
		performObjectSplit(left, right, data, object);
	}

	// Else keep going Deeper.

	//m_numDuplicates += left.numChunks + right.numChunks - spec.numChunks;
	//float progressMid = lerp(progressStart, progressEnd, (float)right.numChunks / (float)(left.numChunks + right.numChunks));
	Node* rightNode = buildNode(right, level + 1);
	Node* leftNode = buildNode(left, level + 1);
	return new BranchNode(data.bounds, leftNode, rightNode);
}

Node* BVH::createLeaf(const NodeData& spec)
{
	auto indices = m_triIndx;



	return nullptr;
}
