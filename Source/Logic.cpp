#include "Logic.hpp"
#include "Message.hpp"
#include "Constants.hpp"
#include "Renderer.hpp"
#include "LensDatabase.hpp"

Logic::Logic()
	: mImageModel(R"(Assets/Models/plane.obj)")
{
	mPlaneVertexBuffer = D3D::getInstance().createHomogenousBuffer(mImageModel.getVertexArray(), DXGI_FORMAT_R32G32B32A32_FLOAT);
	mPlaneTexcoordBuffer = D3D::getInstance().createHomogenousBuffer(mImageModel.getTextCoordArray(), DXGI_FORMAT_R32G32_FLOAT);
	mPlaneIndexBuffer = D3D::getInstance().createHomogenousBuffer(mImageModel.getIndexArray(), DXGI_FORMAT_R32G32B32A32_UINT);
	mPlaneTexture = D3D::getInstance().createBasicTexture(mImageModel.getTexture(), mImageModel.getTextureDimension());

	mSampler = D3D::getInstance().createSampler();
	mRenderTexture = D3D::getInstance().createBasicTexture(std::vector<uint32_t>(), {WIDTH, HEIGHT}, DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE);

	mLensBuffer = D3D::getInstance().createStructuredBuffer(std::vector<LensStruct>(mLensArray.size()));
	
	ConstantBuffer constants{
		mImageModel.getIndexArray().size() / 3,
	};
	
	mConstantBuffer.buffer = D3D::getInstance().createBuffer(constants, D3D11_BIND_CONSTANT_BUFFER);
	
	mVertexShader = D3D::getInstance().createShader<uni::VertexShader>(LR"(Assets\Shaders\vertex.hlsl)", "vs_5_0");
	mPixelShader = D3D::getInstance().createShader<uni::PixelShader>(LR"(Assets\Shaders\pixel.hlsl)", "ps_5_0");
	mRaytraceShader = D3D::getInstance().createShader<uni::ComputeShader>(LR"(Assets\Shaders\raytrace.hlsl)", "cs_5_0");
	
	MessageBus::registerListener(this);


	///////////////////////////////
	// sphereBiconcave(state, float3(0, 4, -4.5), float3(0, 4, 16.5), 10, 10, float3(-2.5, 0.5, 4.5), float3(2.5, 7.5, 7.5));
	// spherePlanoConvex(state, float3(0, 4, 8.5), 10, float3(-2.5, 0.5, -1.5), float3(2.5, 7.5, 1.5));

	auto createBiconcave = [](DirectX::XMFLOAT3 center, DirectX::XMFLOAT2 dimensions, float width, float radius1, float radius2)
	{
		LensStruct lense;
		lense.type = LensType::BICONCAVE;
		lense.radius1 = radius1;
		lense.radius2 = radius2;
		lense.minBox = { center.x - dimensions.x / 2, center.y - dimensions.y / 2, center.z - width };
		lense.maxBox = { center.x + dimensions.x / 2, center.y + dimensions.y / 2, center.z + width };
		lense.center1 = center;
		lense.center2 = center;

		lense.center1.z += width / 2 + radius1;
		lense.center2.z -= width / 2 - radius2;

		return lense;
	};

	auto createPlanoConvex = [](DirectX::XMFLOAT3 center, DirectX::XMFLOAT2 dimensions, float width, float radius)
	{
		LensStruct lense;
		lense.type = LensType::PLANOCONVEX;
		lense.radius1 = radius;
		lense.minBox = { center.x - dimensions.x / 2, center.y - dimensions.y / 2, center.z - width / 2 };
		lense.maxBox = { center.x + dimensions.x / 2, center.y + dimensions.y / 2, center.z + width / 2 };
		lense.center1 = center;

		lense.center1.z = radius + lense.minBox.z;

		return lense;
	};

	auto createBiconvex = [](DirectX::XMFLOAT3 center, DirectX::XMFLOAT2 dimensions, float width, float radius1, float radius2)
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
	};


	LensStruct lens1 = createBiconcave({0, 4, 340}, {5, 7}, 1.5, 10, 10);
	LensStruct lens2 = createPlanoConvex({0, 4, 325}, {5, 7}, 2, 30);
	LensStruct lens3 = createBiconvex({0, 4, 6}, {5, 7}, 1.5, 10, 10);


	pushLense(lens1);
	pushLense(lens2);
	// pushLense(lens3);

	updateLensBuffer();
}

void Logic::update(float dt)
{
	mCamera.update(dt);
	submitDraw();
}

void Logic::recieveMessage(Message message)
{
}

void Logic::pushLense(LensStruct& lense)
{
	mLensArray[mLensCount++] = lense;
}

void Logic::popLense()
{
	mLensCount = mLensCount == 0 ? mLensCount : mLensCount - 1;
}

void Logic::clearLens()
{
	mLensCount = 0;
}

LensStruct& Logic::getLense(size_t index)
{
	return mLensArray[index];
}


void Logic::submitDraw()
{
	auto rt = new RayTraceStruct{
		mCamera.getBufferGPU(),
		mLensBuffer,
		mPlaneVertexBuffer,
		mPlaneTexcoordBuffer,
		mPlaneIndexBuffer,
		mConstantBuffer,
		mPlaneTexture,
		mRenderTexture,
		mSampler,
		mVertexShader,
		mPixelShader,
		mRaytraceShader
	};

	Message msg;
	msg.messageID = MessageID::DRAW_RT;
	msg.datap = rt;

	MessageBus::post(msg);
}

void Logic::updateLensBuffer()
{
	D3D::getInstance().updateBuffer(mLensBuffer.buffer, mLensArray.data());

	
	ConstantBuffer constants{
		mImageModel.getIndexArray().size() / 3,
		mLensCount,
	};
	D3D::getInstance().updateBuffer(mConstantBuffer.buffer, &constants);
}

