#include "Lens.hpp"
#include "Logic.hpp"

Lens::Lens(ConstantBuffer& buffer)
	: mConstantBufferData(buffer)
{
	mLens.reserve(32);
}

LensStruct& Lens::updateLens(LensStruct& lens)
{
	switch (lens.type)
	{
	case LensType::BICONCAVE: createBiconcave(lens); break;
	case LensType::BICONVEX: createBiconvex(lens); break;
	case LensType::PLANOCONVEX: createPlanoConvex(lens); break;
	}

	return reorderLens(lens);
}

void Lens::create(LensStruct&& lens)
{
	mLens.emplace_back(lens);
	mConstantBufferData.lensCount++;
	updateLens(mLens.back());
}

void Lens::createBiconcave(LensStruct& lense)
{
	lense.type = LensType::BICONCAVE;
	//lense.radius1 = radius1;
	//lense.radius2 = radius2;
	lense.minBox = { lense.center.x - lense.dimensions.x / 2, lense.center.y - lense.dimensions.y / 2, lense.center.z - lense.width };
	lense.maxBox = { lense.center.x + lense.dimensions.x / 2, lense.center.y + lense.dimensions.y / 2, lense.center.z + lense.width };
	lense.center1 = lense.center;
	lense.center2 = lense.center;

	lense.center1.z += lense.radius1 + lense.width / 2;
	lense.center2.z -= lense.radius2 + lense.width / 2;
}

void Lens::createBiconvex(LensStruct& lense)
{
	lense.type = LensType::BICONVEX;
	//lense.radius1 = radius1;
	//lense.radius2 = radius2;
	lense.minBox = { lense.center.x - lense.dimensions.x / 2, lense.center.y - lense.dimensions.y / 2, lense.center.z - lense.width / 2 };
	lense.maxBox = { lense.center.x + lense.dimensions.x / 2, lense.center.y + lense.dimensions.y / 2, lense.center.z + lense.width / 2 };
	lense.center1 = lense.center;
	lense.center2 = lense.center;

	lense.center1.z = lense.maxBox.z - lense.radius1;
	lense.center2.z = lense.minBox.z + lense.radius2;
}

void Lens::createPlanoConvex(LensStruct& lense)
{
	lense.type = LensType::PLANOCONVEX;
	//lense.radius1 = radius;
	lense.minBox = { lense.center.x - lense.dimensions.x / 2, lense.center.y - lense.dimensions.y / 2, lense.center.z - lense.width / 2 };
	lense.maxBox = { lense.center.x + lense.dimensions.x / 2, lense.center.y + lense.dimensions.y / 2, lense.center.z + lense.width / 2 };
	lense.center1 = lense.center;

	lense.center1.z = lense.radius1 + lense.minBox.z;
}

LensStruct& Lens::reorderLens(LensStruct& lens)
{
	// reorder based on center
	size_t position = 0;
	for (; &lens != &mLens[position]; ++position);
	
	auto shiftDown = [this, &position, &lens]() { return position > 0 && lens.center.z > mLens[position - 1].center.z; };
	auto shiftUp = [this, &position, &lens]() { return position < mLens.size() - 1 && lens.center.z < mLens[position + 1].center.z; };

	for ( ;shiftDown(); --position)
		std::swap(mLens[position], mLens[position - 1]);

	for ( ;shiftUp(); ++position)
		std::swap(mLens[position], mLens[position + 1]);

	return mLens[position];
}

void Lens::push(LensStruct& lens)
{
	// TODO reorder
	mLens.emplace_back(lens);
	mConstantBufferData.lensCount++;
}

void Lens::pop()
{
	mLens.pop_back();
	mConstantBufferData.lensCount--;
}

void Lens::clear()
{
	mLens.clear();
	mConstantBufferData.lensCount = 0;
}

void Lens::erase(LensStruct& lens)
{
	for (size_t position = 0; ; ++position)
	{
		if (&lens == &mLens[position])
		{
			mLens.erase(mLens.begin() + position);
			mConstantBufferData.lensCount--;
			break;
		}
	}
}

std::vector<LensStruct>& Lens::data()
{
	return mLens;
}

ConstantBuffer& Lens::getConstBuff()
{
	return mConstantBufferData;
}
