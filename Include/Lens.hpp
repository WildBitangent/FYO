#pragma once
#include <DirectXMath.h>


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

namespace Lens
{
	LensStruct createBiconcave(DirectX::XMFLOAT3 center, DirectX::XMFLOAT2 dimensions, float width, float radius1, float radius2);
	LensStruct createBiconvex(DirectX::XMFLOAT3 center, DirectX::XMFLOAT2 dimensions, float width, float radius1, float radius2);
	LensStruct createPlanoConvex(DirectX::XMFLOAT3 center, DirectX::XMFLOAT2 dimensions, float width, float radius);
}
