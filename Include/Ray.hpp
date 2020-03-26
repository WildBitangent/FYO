#pragma once
#include <vector>
#include "Lens.hpp"

struct RayStruct
{
	DirectX::XMFLOAT3 origin;
	DirectX::XMFLOAT3 direction;
};

class Ray
{
	
	std::vector<DirectX::XMFLOAT3> simulate(LensStruct* data, size_t count, DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 direction);

	void interesectBiconcave(LensStruct& data);
	void interesectBiconvex(LensStruct& data);
	void interesectPlanoConvex(LensStruct& data);
};