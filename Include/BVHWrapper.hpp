#pragma once
#include <vector>
#include <DirectXMath.h>
#include "SBVH/Array.h"

struct LensTransform;

class BVHWrapper
{
public:
	struct alignas(16) BVHNode 
	{
		DirectX::XMFLOAT3A min;
		DirectX::XMFLOAT3A max;
		int leftIndex;
		int rightIndex;
		int isLeaf;
	}; 
	
public:
	BVHWrapper() = default;
	BVHWrapper(const std::vector<LensTransform>& lens);

	std::vector<BVHNode>& getTree();
	std::vector<DirectX::XMUINT4>& getIndices();
	std::vector<DirectX::XMFLOAT3A>& getNormals();
	std::vector<DirectX::XMFLOAT3A>& getVertices();
	
private:
	std::vector<BVHNode> mGPUTree;
	std::vector<DirectX::XMUINT4> mIndices;
	std::vector<DirectX::XMFLOAT3A> mNormals;
    std::vector<DirectX::XMFLOAT3A> mVertices;
};