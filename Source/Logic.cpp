#include "Logic.hpp"
#include "Message.hpp"
#include "Constants.hpp"
#include "Renderer.hpp"
#include "LensDatabase.hpp"

Logic::Logic()
	: mImageModel(R"(Assets/Models/plane.obj)")
{
	mConstBufferData.triangleCountPlane = mImageModel.getIndexArray().size() / 3;

	mPlaneVertexBuffer = D3D::getInstance().createHomogenousBuffer(mImageModel.getVertexArray(), DXGI_FORMAT_R32G32B32A32_FLOAT);
	mPlaneTexcoordBuffer = D3D::getInstance().createHomogenousBuffer(mImageModel.getTextCoordArray(), DXGI_FORMAT_R32G32_FLOAT);
	mPlaneIndexBuffer = D3D::getInstance().createHomogenousBuffer(mImageModel.getIndexArray(), DXGI_FORMAT_R32G32B32A32_UINT);
	mPlaneTexture = D3D::getInstance().createBasicTexture(mImageModel.getTexture(), mImageModel.getTextureDimension());

	mSampler = D3D::getInstance().createSampler();
	mRenderTexture = D3D::getInstance().createBasicTexture(std::vector<uint32_t>(), {WIDTH, HEIGHT}, DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE);

	mLensBuffer = D3D::getInstance().createStructuredBuffer(std::vector<LensStruct>(mLensArray.size()));
	mRaysBuffer = D3D::getInstance().createStructuredBuffer(std::vector<RayStruct>(mRayArray.size()));
	//mRaysBuffer = D3D::getInstance().createStructuredBuffer(std::vector<TracedRayStruct>({ { {-1, 3, 30}, {0, 0, -1} }, { { 1, 3, 30 }, { 0, 0, -1 } } }));
	
	mConstantBuffer.buffer = D3D::getInstance().createBuffer(mConstBufferData, D3D11_BIND_CONSTANT_BUFFER);
	
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

		lense.center1.z += radius1 + width / 2;
		lense.center2.z -= radius2 + width / 2;

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


	//LensStruct lens1 = createBiconcave({0, 4, 340}, {5, 7}, 1.5, 10, 10);
	//LensStruct lens2 = createPlanoConvex({0, 4, 325}, {5, 7}, 2, 30);
	//LensStruct lens3 = createBiconvex({0, 4, 6}, {5, 7}, 1.5, 10, 10);

	LensStruct lens1 = createBiconvex({0, 3, 7}, {5, 7}, 1.5, 10, 10);
	LensStruct lens2 = createBiconcave({ 0, 3, 15 }, { 5, 7 }, 1.5, 5, 5);
	LensStruct lens3 = createPlanoConvex({ 0, 3, 25 }, { 5, 7 }, 1.5, 10);


	pushLense(lens3);
	pushLense(lens2);
	pushLense(lens1);

	updateLensBuffer();
}

void Logic::update(float dt)
{
	mCamera.update(dt);
	submitDraw();
}

void Logic::recieveMessage(Message message)
{
	if (message.messageID == MessageID::INPUT_KEY)
	{
		if (message.datai == 'B') // ray beam around
		{
			mConstBufferData.raysCount = 0;
			DirectX::XMFLOAT3 camPos;
			DirectX::XMStoreFloat3(&camPos, mCamera.getBufferCPU()->position);

			constexpr size_t rayCount = 10;
			constexpr float radius = 1.5f;

			for (size_t i = 0; i < rayCount; ++i)
			{
				float theta = (2 * DirectX::XM_PI * i) / rayCount;
				DirectX::XMFLOAT3 origin = {
					radius * std::sinf(theta) + camPos.x,
					radius * std::cosf(theta) + camPos.y,
					camPos.z
				};

				mRayArray[mConstBufferData.raysCount++] = { origin, mCamera.getDirection() };
			}
			updateRaysBuffer();
		}
		else if (message.datai == 'V')
		{
			mConstBufferData.raysCount = 0;
			DirectX::XMFLOAT3 origin;
			DirectX::XMStoreFloat3(&origin, mCamera.getBufferCPU()->position);

			constexpr size_t rayCount = 32;
			constexpr float radius = 0.015f; // max 0.5

			for (size_t i = 0; i < rayCount; ++i)
			{
				float theta = (2 * DirectX::XM_PI * i) / rayCount;

				float x = 0.5f + radius * std::sinf(theta);
				DirectX::XMVECTOR coordX = DirectX::XMVectorSet(x, x, x, x);

				float y = 0.5f + radius * std::cosf(theta);
				DirectX::XMVECTOR coordY = DirectX::XMVectorSet(y, y, y, y);

				DirectX::XMFLOAT3 direction;
				DirectX::XMStoreFloat3(&direction,
					DirectX::XMVector3Normalize(
						DirectX::XMVectorAdd(
							mCamera.getBufferCPU()->upperLeftCorner,
							DirectX::XMVectorSubtract(
								DirectX::XMVectorMultiply(coordX, mCamera.getBufferCPU()->horizontal),
								DirectX::XMVectorMultiply(coordY, mCamera.getBufferCPU()->vertical)
							)
						)
					)
				);

				//if (i == 0)
				//	origin.z -= 0.00001f * (direction.z > 0) ? 1 : -1;
				
				mRayArray[mConstBufferData.raysCount++] = { origin, direction };
			}
			updateRaysBuffer();
		}
	}
}

void Logic::pushLense(LensStruct& lense)
{
	mLensArray[mConstBufferData.lensCount++] = lense;
}

void Logic::popLense()
{
	mConstBufferData.lensCount = mConstBufferData.lensCount == 0 ? mConstBufferData.lensCount : mConstBufferData.lensCount - 1;
}

void Logic::clearLens()
{
	mConstBufferData.lensCount = 0;
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
		mRaysBuffer,
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

	mConstBufferData.lensMinBox = mLensArray[0].minBox;
	mConstBufferData.lensMaxBox = DirectX::XMFLOAT3A(reinterpret_cast<float*>(&mLensArray[0].maxBox));

	// create AABB
	for (size_t i = 1; i < mConstBufferData.lensCount; ++i)
	{
		auto min = [](auto& a, auto& b) {
			a.x = (a.x < b.x ? a.x : b.x);
			a.y = (a.y < b.y ? a.y : b.y);
			a.z = (a.z < b.z ? a.z : b.z);
		};

		auto max = [](auto& a, auto& b) {
			a.x = (a.x > b.x ? a.x : b.x);
			a.y = (a.y > b.y ? a.y : b.y);
			a.z = (a.z > b.z ? a.z : b.z);
		};

		min(mConstBufferData.lensMinBox, mLensArray[i].minBox);
		max(mConstBufferData.lensMaxBox, mLensArray[i].maxBox);
	}
	
	D3D::getInstance().updateBuffer(mConstantBuffer.buffer, &mConstBufferData);
}

void Logic::updateRaysBuffer()
{
	D3D::getInstance().updateBuffer(mRaysBuffer.buffer, mRayArray.data());
	D3D::getInstance().updateBuffer(mConstantBuffer.buffer, &mConstBufferData);
}
