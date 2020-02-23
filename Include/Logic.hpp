#pragma once
#include "Util.hpp"
#include "Model.hpp"
#include "D3D.hpp"
#include "Message.hpp"

class Renderer;

struct alignas(16) ConstantBuffer
{
	uint32_t triangleCountPlane;
	uint32_t triangleCountLens;
	float lensIOR = 1.51714f;
};

class Logic : public Listener
{
public:
	Logic();
	
	void update(float dt);

	void recieveMessage(Message message) override;

private:
	void submitDraw();
	
private:
	Camera mCamera;
	
	Model mImageModel;
	Model mLensModel;

	Buffer mLensVertexBuffer;
	Buffer mLensNormalBuffer;
	Buffer mLensIndexBuffer;
	
	Buffer mPlaneVertexBuffer;
	Buffer mPlaneTexcoordBuffer;
	Buffer mPlaneIndexBuffer;
	
	Buffer mConstantBuffer;

	Texture mPlaneTexture;
	Texture mRenderTexture;
	
	uni::SamplerState mSampler;

	uni::VertexShader mVertexShader;
	uni::PixelShader mPixelShader;
	uni::ComputeShader mRaytraceShader;
};
