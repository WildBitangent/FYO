#pragma once
#include <Windows.h>
#include "RingBuffer.hpp"
#include "Lens.hpp"
#include "Util.hpp"

struct ID3D11Device;
struct ID3D11DeviceContext;

class LensGUI
{
public:
	LensGUI();
	~LensGUI();

	void init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);
	void update(Lens& lens, const std::vector<Sample>& samples);
	void render() const;

private:
	RingBuffer<float, 64> mFPSHistory;
	LensStruct* mSelectedLens = nullptr;
	Sample const* mSelectedSample = nullptr;

	size_t mBeamCount = 8;
	float mBeamRadius = 1.5f;
	//size_t mRaysCount = 8;
	//float mRaysRadius = 0.015f;

	friend class Logic;
};