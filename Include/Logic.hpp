#pragma once
#include "Util.hpp"
#include "Model.hpp"
#include "D3D.hpp"
#include "Message.hpp"
#include "BVHWrapper.hpp"

struct alignas(16) ConstantBuffer
{
	uint32_t triangleCountPlane;
	uint32_t lensCount = 0;
	float lensIOR = 1.51714f;
};

class Logic : public Listener
{
public:
	Logic();
	
	void update(float dt);

	void recieveMessage(Message message) override;

	void pushLense(LensStruct& lense);
	void popLense();
	void clearLens();
	LensStruct& getLense(size_t index);
	
private:
	void submitDraw();
	void updateLensBuffer();
	
private:
	Camera mCamera;

	Model mImageModel;
	std::array<LensStruct, 20> mLensArray;
	size_t mLensCount = 0;
	
	Buffer mPlaneVertexBuffer;
	Buffer mPlaneTexcoordBuffer;
	Buffer mPlaneIndexBuffer;
	
	Buffer mLensBuffer;
	Buffer mConstantBuffer;

	Texture mPlaneTexture;
	Texture mRenderTexture;
	
	uni::SamplerState mSampler;

	uni::VertexShader mVertexShader;
	uni::PixelShader mPixelShader;
	uni::ComputeShader mRaytraceShader;
};
