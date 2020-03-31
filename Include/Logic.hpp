#pragma once
#include "Util.hpp"
#include "Model.hpp"
#include "D3D.hpp"
#include "Message.hpp"
#include "BVHWrapper.hpp"
#include "Lens.hpp"
#include "Ray.hpp"
#include "GUI.hpp"

struct alignas(16) ConstantBuffer
{
	uint32_t triangleCountPlane;
	uint32_t lensCount = 0;
	float lensIOR = 1.51714f;
	uint32_t raysCount = 0;

	DirectX::XMFLOAT3 lensMinBox;
	float lineWidth = 0.025f;

	DirectX::XMFLOAT3 lensMaxBox;
	uint32_t chromaticAberration = 0;
};

class Logic : public Listener
{
public:
	enum MessageID
	{
		UPDATE_LENS,
		UPDATE_BEAM,
		UPDATE_CONST_BUF,
	};

public:
	Logic();
	
	void update(float dt);

	// message system
	void recieveMessage(Message message) override;
	Message* recieveExpressMessage(const Message& message) override;
	
private:
	void submitDraw();
	void updateLensBuffer();
	void updateRaysBuffer();

	void createOrtoBeam(bool update = false);
	void createCameraBeam(bool update = false);
	void createParallelBeam(bool update = false);
	
private:
	Camera mCamera;
	Model mImageModel;
	LensGUI mGUI;

	Lens mLens;
	//std::array<LensStruct, 20> mLensArray;
	std::array<RayStruct, 32> mRayArray;
	size_t mLastBeamType = 'B'; // todo mby type safety

	std::vector<Sample> mSamples;

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
