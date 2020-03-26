#include "Lens.hpp"

using namespace Lens;

LensStruct createBiconcave(DirectX::XMFLOAT3 center, DirectX::XMFLOAT2 dimensions, float width, float radius1, float radius2)
{
	LensStruct lense;
	lense.type = LensType::BICONCAVE;
	lense.radius1 = radius1;
	lense.radius2 = radius2;
	lense.minBox = { center.x - dimensions.x / 2, center.y - dimensions.y / 2, center.z - width };
	lense.maxBox = { center.x + dimensions.x / 2, center.y + dimensions.y / 2, center.z + width };
	lense.center1 = center;
	lense.center2 = center;

	lense.center1.z += radius1 + width / 2;
	lense.center2.z -= radius2 + width / 2;

	return lense;
}

LensStruct createBiconvex(DirectX::XMFLOAT3 center, DirectX::XMFLOAT2 dimensions, float width, float radius1, float radius2)
{
	LensStruct lense;
	lense.type = LensType::BICONVEX;
	lense.radius1 = radius1;
	lense.radius2 = radius2;
	lense.minBox = { center.x - dimensions.x / 2, center.y - dimensions.y / 2, center.z - width / 2 };
	lense.maxBox = { center.x + dimensions.x / 2, center.y + dimensions.y / 2, center.z + width / 2 };
	lense.center1 = center;
	lense.center2 = center;

	lense.center1.z = lense.maxBox.z - radius1;
	lense.center2.z = lense.minBox.z + radius2;

	return lense;
}

LensStruct createPlanoConvex(DirectX::XMFLOAT3 center, DirectX::XMFLOAT2 dimensions, float width, float radius)
{
	LensStruct lense;
	lense.type = LensType::PLANOCONVEX;
	lense.radius1 = radius;
	lense.minBox = { center.x - dimensions.x / 2, center.y - dimensions.y / 2, center.z - width / 2 };
	lense.maxBox = { center.x + dimensions.x / 2, center.y + dimensions.y / 2, center.z + width / 2 };
	lense.center1 = center;

	lense.center1.z = radius + lense.minBox.z;

	return lense;
}


