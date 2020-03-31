#include "Logic.hpp"
#include "Message.hpp"
#include "Constants.hpp"
#include "Renderer.hpp"
#include "LensDatabase.hpp"
#include "Samples.inl"

Logic::Logic()
	: mImageModel(R"(Assets/Models/plane.obj)")
	, mLens(mConstBufferData)
{
	mConstBufferData.triangleCountPlane = mImageModel.getIndexArray().size() / 3;

	mPlaneVertexBuffer = D3D::getInstance().createHomogenousBuffer(mImageModel.getVertexArray(), DXGI_FORMAT_R32G32B32A32_FLOAT);
	mPlaneTexcoordBuffer = D3D::getInstance().createHomogenousBuffer(mImageModel.getTextCoordArray(), DXGI_FORMAT_R32G32_FLOAT);
	mPlaneIndexBuffer = D3D::getInstance().createHomogenousBuffer(mImageModel.getIndexArray(), DXGI_FORMAT_R32G32B32A32_UINT);
	mPlaneTexture = D3D::getInstance().createBasicTexture(mImageModel.getTexture(), mImageModel.getTextureDimension());

	mSampler = D3D::getInstance().createSampler();
	mRenderTexture = D3D::getInstance().createBasicTexture(std::vector<uint32_t>(), {WIDTH, HEIGHT}, DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE);

	mLensBuffer = D3D::getInstance().createStructuredBuffer(std::vector<LensStruct>(32)); // TODO: magic constant
	mRaysBuffer = D3D::getInstance().createStructuredBuffer(std::vector<RayStruct>(mRayArray.size()));
	
	mConstantBuffer.buffer = D3D::getInstance().createBuffer(mConstBufferData, D3D11_BIND_CONSTANT_BUFFER);
	
	mVertexShader = D3D::getInstance().createShader<uni::VertexShader>(LR"(Assets\Shaders\vertex.hlsl)", "vs_5_0");
	mPixelShader = D3D::getInstance().createShader<uni::PixelShader>(LR"(Assets\Shaders\pixel.hlsl)", "ps_5_0");
	mRaytraceShader = D3D::getInstance().createShader<uni::ComputeShader>(LR"(Assets\Shaders\raytrace.hlsl)", "cs_5_0");
	
	MessageBus::registerListener(this);

	initSamples(mSamples);

	mSamples[0].funct(mLens);
	updateLensBuffer();
}

void Logic::update(float dt)
{
	mCamera.update(dt);
	mGUI.update(mLens, mSamples);
	submitDraw();
}

void Logic::recieveMessage(Message message)
{
	if (message.messageID == ::MessageID::INPUT_KEY)
	{
		switch (message.datai)
		{
		case 'B': createOrtoBeam(); mLastBeamType = 'B'; break;
		case 'V': createCameraBeam(); mLastBeamType = 'V'; break;
		case 'C': createParallelBeam(); mLastBeamType = 'C'; break;
		}
	}
	else if (message.messageID == ::MessageID::LOGIC)
	{
		if (message.datai == MessageID::UPDATE_CONST_BUF)
			D3D::getInstance().updateBuffer(mConstantBuffer.buffer, &mConstBufferData);
		else if (message.datai == MessageID::UPDATE_BEAM)
		{
			switch (mLastBeamType)
			{
			case 'B': createOrtoBeam(true); break;
			case 'V': createCameraBeam(true); break;
			case 'C': createParallelBeam(true); break;
			}
		}
		else if (message.datai == MessageID::UPDATE_LENS)
			updateLensBuffer();
	}
}

Message* Logic::recieveExpressMessage(const Message& message)
{
	return nullptr;
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

	MessageBus::post({ ::MessageID::DRAW_RT, {.datap = rt } });
	MessageBus::post({ ::MessageID::DRAW_GUI, {.datap = &mGUI } });
}

void Logic::updateLensBuffer()
{
	D3D::getInstance().updateBuffer(mLensBuffer.buffer, mLens.data().data());

	if (!mLens.data().empty())
	{
		mConstBufferData.lensMinBox = mLens.data().begin()->minBox;
		mConstBufferData.lensMaxBox = DirectX::XMFLOAT3A(reinterpret_cast<float*>(&mLens.data().begin()->maxBox));

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

			min(mConstBufferData.lensMinBox, mLens.data()[i].minBox);
			max(mConstBufferData.lensMaxBox, mLens.data()[i].maxBox);
		}
	}
	
	D3D::getInstance().updateBuffer(mConstantBuffer.buffer, &mConstBufferData);
}

void Logic::updateRaysBuffer()
{
	D3D::getInstance().updateBuffer(mRaysBuffer.buffer, mRayArray.data());
	D3D::getInstance().updateBuffer(mConstantBuffer.buffer, &mConstBufferData);
}

void Logic::createOrtoBeam(bool update)
{
	static DirectX::XMFLOAT3 lastCamPos;
	static DirectX::XMFLOAT3 lastDirection;

	mConstBufferData.raysCount = 0;
	DirectX::XMFLOAT3 camPos;
	DirectX::XMFLOAT3 direction;

	if (update)
	{
		camPos = lastCamPos;
		direction = lastDirection;
	}
	else
	{
		DirectX::XMStoreFloat3(&camPos, mCamera.getBufferCPU()->position);
		direction = mCamera.getDirection();
	}

	for (size_t i = 0; i < mGUI.mBeamCount; ++i)
	{
		float theta = (2 * DirectX::XM_PI * i) / mGUI.mBeamCount;
		DirectX::XMFLOAT3 origin = {
			mGUI.mBeamRadius * std::sinf(theta) + camPos.x,
			mGUI.mBeamRadius * std::cosf(theta) + camPos.y,
			camPos.z
		};

		mRayArray[mConstBufferData.raysCount++] = { origin, direction };
	}
	updateRaysBuffer();
	
	if (!update)
	{
		lastCamPos = camPos;
		lastDirection = mCamera.getDirection();
	}
}

void Logic::createCameraBeam(bool update)
{
	static Camera::CameraBuffer lastCamera;

	mConstBufferData.raysCount = 0;
	DirectX::XMFLOAT3 origin;

	DirectX::XMStoreFloat3(&origin, (update ? lastCamera.position : mCamera.getBufferCPU()->position));

	float radius = 1.f - std::expf(0.01f * mGUI.mBeamRadius);
	for (size_t i = 0; i < mGUI.mBeamCount; ++i)
	{
		float theta = (2 * DirectX::XM_PI * i) / mGUI.mBeamCount;

		float x = 0.5f + radius * std::sinf(theta);
		DirectX::XMVECTOR coordX = DirectX::XMVectorSet(x, x, x, x);

		float y = 0.5f + radius * std::cosf(theta);
		DirectX::XMVECTOR coordY = DirectX::XMVectorSet(y, y, y, y);

		DirectX::XMFLOAT3 direction;
		DirectX::XMStoreFloat3(&direction,
			DirectX::XMVector3Normalize(
				DirectX::XMVectorAdd(
					(update ? lastCamera.upperLeftCorner : mCamera.getBufferCPU()->upperLeftCorner),
					DirectX::XMVectorSubtract(
						DirectX::XMVectorMultiply(coordX, (update ? lastCamera.horizontal : mCamera.getBufferCPU()->horizontal)),
						DirectX::XMVectorMultiply(coordY, (update ? lastCamera.vertical : mCamera.getBufferCPU()->vertical))
					)
				)
			)
		);

		mRayArray[mConstBufferData.raysCount++] = { origin, direction };
	}
	updateRaysBuffer();

	if (!update)
		lastCamera = *mCamera.getBufferCPU();
}

void Logic::createParallelBeam(bool update)
{
	static DirectX::XMFLOAT3 lastCamPos;
	static DirectX::XMFLOAT3 lastDirection;

	mConstBufferData.raysCount = 0;
	DirectX::XMFLOAT3 camPos;
	DirectX::XMFLOAT3 direction;

	if (update)
	{
		camPos = lastCamPos;
		direction = lastDirection;
	}
	else
	{
		DirectX::XMStoreFloat3(&camPos, mCamera.getBufferCPU()->position);
		direction = mCamera.getDirection();
	}

	float start = camPos.y - (mGUI.mBeamRadius);
	float offset = (mGUI.mBeamRadius * 2) / (mGUI.mBeamCount - 1);
	for (size_t i = 0; i < mGUI.mBeamCount; ++i)
	{
		mRayArray[mConstBufferData.raysCount++] = { { camPos.x, start + offset, camPos.z }, direction };
		start += offset;
	}
	updateRaysBuffer();

	if (!update)
	{
		lastCamPos = camPos;
		lastDirection = mCamera.getDirection();
	}

}
