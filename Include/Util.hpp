#pragma once
#include <utility>
#include "UniqueDX11.hpp"
#include "DirectXMath.h"
#include "D3D.hpp"
#include "Camera.hpp"

class Lens;

struct RayTraceStruct
{
	Buffer& camera;
	Buffer& lensArray;
	Buffer& raysArray;
	
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

struct Sample
{
	std::string name;
	void(*funct)(Lens&);
};
