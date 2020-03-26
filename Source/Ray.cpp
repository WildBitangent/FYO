#include "Ray.hpp"

using namespace DirectX;

XMVECTOR glassSample(XMVECTOR rayDirection, XMVECTOR normal, float IOR, bool rayToMat)
{
	float n1 = 1.0;
	float n2 = IOR;

	// decide where do we go, inside or outside
	float refractFactor = (rayToMat ? n1 / n2 : n2 / n1);
	XMVECTOR transDirection = XMVector3Normalize(XMVector3Refract(rayDirection, normal, refractFactor));

	return transDirection;
}

std::vector<DirectX::XMFLOAT3> Ray::simulate(LensStruct* data, size_t count, DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 direction)
{
	return std::vector<DirectX::XMFLOAT3>();
}

void Ray::interesectBiconcave(LensStruct& data)
{

}

void Ray::interesectBiconvex(LensStruct& data)
{

}

void Ray::interesectPlanoConvex(LensStruct& data)
{

}
