#include "BVHWrapper.hpp"
#include "SBVH/BVH.h"
#include <stack>
#include "LensDatabase.hpp"

// todo lens database


BVHWrapper::BVHWrapper(const std::vector<LensTransform>& lens)
{
	Array<GPUScene::Triangle> triangles;
	Array<Vec3f> vertices;

	for (const auto& l : lens)
	{
		const auto offset = vertices.getSize();

		for (auto v : l.model.getVertexArray())
		{
			XMStoreFloat3A(&v, XMVector3Transform(XMLoadFloat3A(&v), l.transform));
			vertices.add(reinterpret_cast<Vec3f&>(v));
			mVertices.emplace_back(v);
		}
		
		for (auto& n : l.model.getNormalArray())
			mNormals.emplace_back(n);

		for (size_t i = 0; i < l.model.getIndexArray().size() / 3; ++i)
		{
			auto& vertex = l.model.getIndexArray()[i];
			auto& normal = l.model.getIndexArray()[i + l.model.getIndexArray().size() / 3];
			Vec3i vi(vertex.x + offset, vertex.y + offset, vertex.z + offset);
			Vec3i ni(normal.x + offset, normal.y + offset, normal.z + offset);

			triangles.add(GPUScene::Triangle(vi, ni));
		}
		
		// for (auto& i : l.model.getIndexArray())
		// 	triangles.add(GPUScene::Triangle(Vec3i(i.x + offset, i.y + offset, i.z + offset)));
	}
	
    GPUScene scene = GPUScene(triangles.getSize(), vertices.getSize(), triangles, vertices);
	
   	const Platform defaultPlatform;
	const BVH::BuildParams defaultParams;
	BVH bvh(&scene, defaultPlatform, defaultParams);

	std::stack<std::pair<::BVHNode*, size_t>> stack{ { {bvh.getRoot(), 0} } };
	mGPUTree.resize(bvh.getNumNodes());

	std::vector<DirectX::XMUINT4> normalIndices;
	for (size_t nodeIndex = 0; !stack.empty(); )
	{
		auto [root, currentIndex] = stack.top(); stack.pop();
		auto& aabb = root->m_bounds;
		auto& node = mGPUTree[currentIndex];
		node.min = { aabb.min().x, aabb.min().y, aabb.min().z };
		node.max = { aabb.max().x, aabb.max().y, aabb.max().z };
		node.isLeaf = false;
				
		if (root->isLeaf())
		{
			const auto leaf = reinterpret_cast<const LeafNode*>(root);
			const auto start = mIndices.size();

			node.leftIndex = start;
			node.rightIndex = start + leaf->m_hi - leaf->m_lo;
			node.isLeaf = true;

			for (auto i = leaf->m_lo; i < leaf->m_hi; i++)
			{
				auto& triangle = bvh.getScene()->getTriangle(bvh.getTriIndices()[i]);
				mIndices.emplace_back(reinterpret_cast<const uint32_t*>(&triangle.vertices));
				normalIndices.emplace_back(reinterpret_cast<const uint32_t*>(&triangle.normals));
			}
		}
		else
		{
			nodeIndex += 2;
			stack.push({ root->getChildNode(1), nodeIndex });
			node.rightIndex = stack.top().second;
			stack.push({ root->getChildNode(0), nodeIndex - 1 });
			node.leftIndex = stack.top().second;
		}

		mGPUTree.emplace_back(node);
	}

	mIndices.insert(mIndices.end(), normalIndices.begin(), normalIndices.end());
}

std::vector<BVHWrapper::BVHNode>& BVHWrapper::getTree()
{
	return mGPUTree;
}

std::vector<DirectX::XMUINT4>& BVHWrapper::getIndices()
{
	return mIndices;
}

std::vector<DirectX::XMFLOAT3A>& BVHWrapper::getNormals()
{
	return mNormals;
}

std::vector<DirectX::XMFLOAT3A>& BVHWrapper::getVertices()
{
	return mVertices;
}
