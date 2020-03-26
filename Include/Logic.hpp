#pragma once
#include "Util.hpp"
#include "Model.hpp"
#include "D3D.hpp"
#include "Message.hpp"
#include "BVHWrapper.hpp"
#include "Lens.hpp"
#include "Ray.hpp"

struct alignas(16) ConstantBuffer
{
	uint32_t triangleCountPlane;
	uint32_t lensCount = 0;
	float lensIOR = 1.51714f;
	uint32_t raysCount = 0;
	DirectX::XMFLOAT3A lensMinBox;
	DirectX::XMFLOAT3A lensMaxBox;
};

class Logic : public Listener
{
public:
	Logic();
	
	void update(float dt);

	void recieveMessage(Message message) override;

	// todo create template class for this
	void pushLense(LensStruct& lense);
	void popLense();
	void clearLens();

	LensStruct& getLense(size_t index);
	
private:
	void submitDraw();
	void updateLensBuffer();
	void updateRaysBuffer();
	
private:
	Camera mCamera;

	Model mImageModel;
	std::array<LensStruct, 20> mLensArray;
	std::array<RayStruct, 32> mRayArray;

	ConstantBuffer mConstBufferData;
	
	Buffer mPlaneVertexBuffer;
	Buffer mPlaneTexcoordBuffer;
	Buffer mPlaneIndexBuffer;
	
	Buffer mLensBuffer;
	Buffer mRaysBuffer;
	Buffer mConstantBuffer;

	Texture mPlaneTexture;
	Texture mRenderTexture;
	
	uni::SamplerState mSampler;

	uni::VertexShader mVertexShader;
	uni::PixelShader mPixelShader;
	uni::ComputeShader mRaytraceShader;
};
