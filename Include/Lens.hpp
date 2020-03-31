#pragma once
#include <DirectXMath.h>
#include <vector>

struct ConstantBuffer;

enum class LensType : uint32_t
{
	BICONCAVE,
	BICONVEX,
	PLANOCONVEX,

	COUNT,
};

inline const char* lensToString(LensType type)
{
	switch (type)
	{
	case LensType::BICONCAVE: return "Biconcave";
	case LensType::BICONVEX: return "Biconvex";
	case LensType::PLANOCONVEX: return "Plano-Convex";
	default: return "";
	}
}

struct alignas(16) LensStruct
{
	DirectX::XMFLOAT3 center1;
	float radius1 = 10.f;
	DirectX::XMFLOAT3 center2;
	float radius2 = 10.f;

	DirectX::XMFLOAT3A minBox;
	DirectX::XMFLOAT3 maxBox;

	LensType type = LensType::BICONVEX;

	// these are just helpers
	// TODO refactor
	DirectX::XMFLOAT3 center = { 0, 3, 0 };
	float width = 1.5f;
	DirectX::XMFLOAT2A dimensions = { 5, 5 };
	
	operator const char* () const
	{
		return lensToString(type);
	}
};

class Lens
{
public:
	Lens(ConstantBuffer& buffer);

	// returns pointer to new location if reorder happens
	LensStruct& updateLens(LensStruct& lens);
	void create(LensStruct&& lens);

	void push(LensStruct& lens);
	void pop();
	void clear();
	void erase(LensStruct& lens);
	std::vector<LensStruct>& data();

	ConstantBuffer& getConstBuff();

private:
	void createBiconcave(LensStruct& lense);
	void createBiconvex(LensStruct& lense);
	void createPlanoConvex(LensStruct& lense);

	LensStruct& reorderLens(LensStruct& lens);

private:
	std::vector<LensStruct> mLens;
	ConstantBuffer& mConstantBufferData;
};

