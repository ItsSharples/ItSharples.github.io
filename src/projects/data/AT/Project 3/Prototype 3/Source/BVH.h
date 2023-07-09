#pragma once
#include <algorithm>
#include "MyMaths.h"
#include "Shape.h"
using Vec3f = MyMaths::Vec3f;

#define getTriangleCost(x) (int)x

using uint = unsigned int;
struct Bounds
{
    Vec3f max, min;

    bool isInside(const Bounds& other)
    {
        auto mx = min < other.max&& other.max < max;
        auto mn = min < other.min&& other.min < max;
        return mx || mn;
    }
    inline void grow(const Vec3f& point) { min = std::min(min, point); max = std::max(max, point); }
    inline void grow(const Bounds& bounds) { grow(bounds.min); grow(bounds.max); }
    inline void intersect(const Bounds& bounds) { min = std::min(min, bounds.min); max = std::max(max, bounds.max); } /// box formed by intersection of 2
    inline Vec3f midPoint() const { return (min + max) * 0.5f; } // centroid or midpoint
    inline float volume() const { if (!valid()) return 0.0f; return (max.x - min.x) * (max.y - min.y) * (max.z - min.z); }
    inline float area() const { Vec3f delta = max - min; return (delta.x * delta.y + delta.y * delta.z + delta.z * delta.x) * 2.0f; }
    inline bool valid() const { return min < max; }
};

struct Chunk // The Smallest Possible thing in the BVH (An Triangle)
{
	uint index; // Index to the Triangle in the BVH Object
	Bounds bounds;
};

struct NodeData
{
    uint numChunks; // The number of Chunks this Node Contains
    Bounds bounds; // It's Bounds
};

struct ObjectSplit
{
    float cost;
    int splitAxis;
    uint numberChunksLeft;
    Bounds leftBounds;
    Bounds rightBounds;

};

struct SpatialSplit
{
    float cost;
    int splitAxis;
    float position; // Position along the splitAxis

};

class Node
{
public:
    Node() = default;
    virtual bool isLeaf() const = 0;
    virtual int getNumChildNodes() const = 0;
    virtual Node* getChildNode(int i) const = 0;
    virtual int getNumTriangles() const { return 0; }

    float getArea() const { return m_bounds.area(); }
    virtual bool intersect(const Vec3f& rayorig, const Vec3f& raydir, float& t0, float& t1) const = 0;

    Bounds m_bounds;
};

class BranchNode : public Node
{
public:
    BranchNode(const Bounds& bounds, Node* childLeft, Node* childRight) { m_bounds = bounds; m_children[0] = childLeft; m_children[1] = childRight; }

    bool        isLeaf() const { return false; }
    int         getNumChildNodes() const { return 2; }
    Node*    getChildNode(int i) const { assert(i >= 0 && i < 2); return m_children[i]; }

    Node* m_children[2];

    bool intersect(const Vec3f& rayorig, const Vec3f& raydir, float& t0, float& t1) const
    {
        return (m_children[0]->intersect(rayorig, raydir, t0, t1) || m_children[1]->intersect(rayorig, raydir, t0, t1));
    }

};

class LeafNode : public Node
{
public:
    LeafNode(const Bounds& bounds, std::vector<Triangle> triangles) { m_bounds = bounds; m_triangles = triangles; }
    LeafNode(const LeafNode& s) { *this = s; }

    bool isLeaf() const { return true; }
    int getNumChildNodes() const { return 0; }  // leafnode has 0 children
    Node* getChildNode(int) const { return NULL; }

    int getNumTriangles() const { return m_triangles.size(); }
    std::vector<Triangle> m_triangles;

    bool intersect(const Vec3f& rayorig, const Vec3f& raydir, float& t0, float& t1) const
    {
        for (auto triangle : m_triangles)
        {
            if (triangle.intersect(rayorig, raydir, t0, t1))
            {
                return true;
            }
        }
        return false;
    }

};




class BVH
{
public:

    BVH(const TriOBJ& tris) :
        m_numTris(tris.indices.size()), m_numVerts(tris.vertices.size()),
        m_tris (tris.vertices), m_triIndx (tris.indices)
    {
        for (auto tri : m_tris)
        {
            m_vertices.emplace_back(tri.x); m_vertices.emplace_back(tri.y); m_vertices.emplace_back(tri.z);
        }

        NodeData rootNode;
        available_chunks.resize(m_numTris);

        for (int index = 0; index < m_numTris; index++)
        {
            available_chunks[index].index = index;

            available_chunks[index].bounds.grow(m_tris[index].x);
            available_chunks[index].bounds.grow(m_tris[index].y);
            available_chunks[index].bounds.grow(m_tris[index].z);

            rootNode.bounds.grow(available_chunks[index].bounds);
        }

        root = std::make_unique<Node>(buildNode(rootNode, 0));
    }
    
    Node* buildNode(const NodeData& spec, int level);
    Node* createLeaf(const NodeData& spec);

private:
    size_t m_numTris;
    size_t m_numVerts;
    std::unique_ptr<Node> root;
    std::vector<Chunk> available_chunks;
    std::vector<Triangle> m_tris;
    std::vector<TriIndex> m_triIndx;
    std::vector<Vec3f> m_vertices;

	using Float = float;
    ObjectSplit findObjectSplit(const NodeSpec& spec, Float nodeSAH)
    {
        ObjectSplit split;
        const Chunk* refPtr = chunkStack.getPtr(chunkStack.size() - spec.numRef);

        // Sort along each dimension.
        for (m_sortDim = 0; m_sortDim < 3; m_sortDim++)
        {
            Sort(chunkStack.size() - spec.numRef, chunkStack.size(), this, sortCompare, sortSwap);

            // Sweep right to left and determine bounds.

            Bounds rightBounds;
            for (int i = spec.numRef - 1; i > 0; i--)
            {
                rightBounds.grow(refPtr[i].bounds);
                rightBounds[i - 1] = rightBounds;
            }

            // Sweep left to right and select lowest SAH.

			Bounds leftBounds;
            for (int i = 1; i < spec.numRef; i++)
            {
                leftBounds.grow(refPtr[i - 1].bounds);
                Float sah = nodeSAH + leftBounds.area() * getTriangleCost(i) + rightBounds[i - 1].area() * getTriangleCost(spec.numRef - i);
                if (sah < split.cost)
                {
                    split.cost = sah;
                    split.splitAxis = m_sortDim;
                    split.numberChunksLeft = i;
                    split.leftBounds = leftBounds;
                    split.rightBounds = m_rightBounds[i - 1];
                }
            }
        }
        return split;
    }

    //------------------------------------------------------------------------

    void performObjectSplit(NodeSpec& left, NodeSpec& right, const NodeSpec& spec, const ObjectSplit& split)
    {
        m_sortDim = split.sortDim;
        Sort(m_refStack.getSize() - spec.numRef, m_refStack.getSize(), this, sortCompare, sortSwap);

        left.numRef = split.numLeft;
        left.bounds = split.leftBounds;
        right.numRef = spec.numRef - split.numLeft;
        right.bounds = split.rightBounds;
    }

	SpatialSplit findSpatialSplit(const NodeSpec& spec, Float nodeSAH)
	{
		// Initialize bins.

		Vec3f origin = spec.bounds.min();
		Vec3f binSize = (spec.bounds.max() - origin) * (1.0f / (Float)NumSpatialBins);
		Vec3f invBinSize = Vec3f(1.0f / binSize.x, 1.0f / binSize.y, 1.0f / binSize.z);

		for (int dim = 0; dim < 3; dim++)
		{
			for (int i = 0; i < NumSpatialBins; i++)
			{
				SpatialBin& bin = m_bins[dim][i];
				bin.bounds = Bounds();
				bin.enter = 0;
				bin.exit = 0;
			}
		}

		// Chop references into bins.
		for (int refIdx = m_refStack.size() - spec.numRef; refIdx < m_refStack.getSize(); refIdx++)
		{
			const Reference& ref = m_refStack[refIdx];

			Vec3<int> firstBin = clamp3i(Vec3<int>((ref.bounds.min() - origin) * invBinSize), Vec3<int>(0, 0, 0), Vec3<int>(NumSpatialBins - 1, NumSpatialBins - 1, NumSpatialBins - 1));
			Vec3<int> lastBin = clamp3i(Vec3<int>((ref.bounds.max() - origin) * invBinSize), firstBin, Vec3<int>(NumSpatialBins - 1, NumSpatialBins - 1, NumSpatialBins - 1));

			for (int dim = 0; dim < 3; dim++)
			{
				Reference currRef = ref;
				for (int i = firstBin._v[dim]; i < lastBin[dim]; i++)
				{
					Reference leftRef, rightRef;
					splitReference(leftRef, rightRef, currRef, dim, origin[dim] + binSize[dim] * (Float)(i + 1));
					m_bins[dim][i].bounds.grow(leftRef.bounds);
					currRef = rightRef;
				}
				m_bins[dim][lastBin._v[dim]].bounds.grow(currRef.bounds);
				m_bins[dim][firstBin._v[dim]].enter++;
				m_bins[dim][lastBin._v[dim]].exit++;
			}
		}

		// Select best split plane.

		SpatialSplit split;
		for (int dim = 0; dim < 3; dim++)
		{
			// Sweep right to left and determine bounds.
			Bounds rightBounds;
			for (int i = NumSpatialBins - 1; i > 0; i--)
			{
				rightBounds.grow(m_bins[dim][i].bounds);
				m_rightBounds[i - 1] = rightBounds;
			}

			// Sweep left to right and select lowest SAH.

			Bounds leftBounds;
			int leftNum = 0;
			int rightNum = spec.numRef;

			for (int i = 1; i < NumSpatialBins; i++)
			{
				leftBounds.grow(m_bins[dim][i - 1].bounds);
				leftNum += m_bins[dim][i - 1].enter;
				rightNum -= m_bins[dim][i - 1].exit;

				Float sah = nodeSAH + leftBounds.area() * getTriangleCost(leftNum) + m_rightBounds[i - 1].area() * getTriangleCost(rightNum);
				if (sah < split.cost)
				{
					split.cost = sah;
					split.splitAxis = dim;
					split.position = origin[dim] + binSize[dim] * (Float)i;
				}
			}
		}
		return split;
	}

	//------------------------------------------------------------------------

	void performSpatialSplit(NodeSpec& left, NodeSpec& right, const NodeSpec& spec, const SpatialSplit& split)
	{
		// Categorize references and compute bounds.
		//
		// Left-hand side:      [leftStart, leftEnd[
		// Uncategorized/split: [leftEnd, rightStart[
		// Right-hand side:     [rightStart, refs.getSize()[

		std::vector<Chunk>& refs;// = //; m_refStack;
		int leftStart = refs.size() - spec.numRef;
		int leftEnd = leftStart;
		int rightStart = refs.size();
		left.bounds = right.bounds = Bounds();

		for (int i = leftEnd; i < rightStart; i++)
		{
			// Entirely on the left-hand side?

			if (refs[i].bounds.max()._v[split.dim] <= split.pos)
			{
				left.bounds.grow(refs[i].bounds);
				std::swap(refs[i], refs[leftEnd++]);
			}

			// Entirely on the right-hand side?

			else if (refs[i].bounds.min()._v[split.dim] >= split.pos)
			{
				right.bounds.grow(refs[i].bounds);
				swap(refs[i--], refs[--rightStart]);
			}
		}

		// Duplicate or unsplit references intersecting both sides.

		while (leftEnd < rightStart)
		{
			// Split reference.

			Reference lref, rref;
			splitReference(lref, rref, refs[leftEnd], split.dim, split.pos);

			// Compute SAH for duplicate/unsplit candidates.

			Bounds lub = left.bounds;  // Unsplit to left:     new left-hand bounds.
			Bounds rub = right.bounds; // Unsplit to right:    new right-hand bounds.
			Bounds ldb = left.bounds;  // Duplicate:           new left-hand bounds.
			Bounds rdb = right.bounds; // Duplicate:           new right-hand bounds.
			lub.grow(refs[leftEnd].bounds);
			rub.grow(refs[leftEnd].bounds);
			ldb.grow(lref.bounds);
			rdb.grow(rref.bounds);

			Float lac = m_platform.getTriangleCost(leftEnd - leftStart);
			Float rac = m_platform.getTriangleCost(refs.getSize() - rightStart);
			Float lbc = m_platform.getTriangleCost(leftEnd - leftStart + 1);
			Float rbc = m_platform.getTriangleCost(refs.getSize() - rightStart + 1);

			Float unsplitLeftSAH = lub.area() * lbc + right.bounds.area() * rac;
			Float unsplitRightSAH = left.bounds.area() * lac + rub.area() * rbc;
			Float duplicateSAH = ldb.area() * lbc + rdb.area() * rbc;
			Float minSAH = min1f3(unsplitLeftSAH, unsplitRightSAH, duplicateSAH);

			// Unsplit to left?

			if (minSAH == unsplitLeftSAH)
			{
				left.bounds = lub;
				leftEnd++;
			}

			// Unsplit to right?

			else if (minSAH == unsplitRightSAH)
			{
				right.bounds = rub;
				swap(refs[leftEnd], refs[--rightStart]);
			}

			// Duplicate?

			else
			{
				left.bounds = ldb;
				right.bounds = rdb;
				refs[leftEnd++] = lref;
				refs.add(rref);
			}
		}

		left.numRef = leftEnd - leftStart;
		right.numRef = refs.getSize() - rightStart;
	}


	Vec3f lerp(const Vec3f& a, const Vec3f& b, const float& t) const
	{
		return a + (b - a) * t;
	}

	void splitReference(Chunk& left, Chunk& right, const Chunk& ref, int dim, float pos)
	{
		// Initialize references.

		left.index = right.index = ref.index;
		left.bounds = right.bounds = Bounds();

		// Loop over vertices/edges.
		const Vec3<int>& inds = 0;// m_triIndx;
		const Vec3f* verts = verts;// m_bvh.getScene()->getVertexPtr();
		Vec3f v1 = verts[inds.z];

		for (int i = 0; i < 3; i++)
		{
			const Vec3f v0 = v1;
			v1 = verts[inds[i]];
			float v0p = v0[dim];
			float v1p = v1[dim];

			// Insert vertex to the boxes it belongs to.

			if (v0p <= pos)
				left.bounds.grow(v0);
			if (v0p >= pos)
				right.bounds.grow(v0);

			// Edge intersects the plane => insert intersection to both boxes.

			if ((v0p < pos && v1p > pos) || (v0p > pos && v1p < pos))
			{
				Vec3f t = lerp(v0, v1, std::clamp((pos - v0p) / (v1p - v0p), 0.0f, 1.0f));
				left.bounds.grow(t);
				right.bounds.grow(t);
			}
		}

		// Intersect with original bounds.

		left.bounds.max[dim] = pos;
		right.bounds.min[dim] = pos;
		left.bounds.intersect(ref.bounds);
		right.bounds.intersect(ref.bounds);
	}
};

