#pragma once
#include <utility>
#include "UniqueDX11.hpp"
#include "DirectXMath.h"
#include "D3D.hpp"
#include "Camera.hpp"

struct RayTraceStruct
{
	Buffer& camera;
	Buffer& lensArray;
	
	Buffer& planeVertexBuffer;
	Buffer& planeTexcoordBuffer;
	Buffer& planeIndexBuffer;
	
	Buffer& constantBuffer;
	
	Texture& planeTexture;
	Texture& renderTexture;

	uni::SamplerState& sampler;

	uni::VertexShader& vertexShader;
	uni::PixelShader& pixelShader;
	uni::ComputeShader& raytraceShader;
};

enum class LensType : uint32_t
{
	BICONCAVE,
	BICONVEX,
	PLANOCONVEX,
};

struct alignas(16) LensStruct
{
	DirectX::XMFLOAT3 center1;
	float radius1;
	DirectX::XMFLOAT3 center2;
	float radius2;

	DirectX::XMFLOAT3A minBox;
	DirectX::XMFLOAT3 maxBox;

	LensType type;
};