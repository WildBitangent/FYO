#include "Logic.hpp"
#include "Message.hpp"
#include "Constants.hpp"
#include "Renderer.hpp"
#include "LensDatabase.hpp"

Logic::Logic()
	: mImageModel(R"(Assets/Models/plane.obj)")
{
	LensDatabase::loadModel(R"(Assets/Models/lens.obj)");
	
	mPlaneVertexBuffer = D3D::getInstance().createStructuredBuffer(mImageModel.getVertexArray());
	mPlaneTexcoordBuffer = D3D::getInstance().createStructuredBuffer(mImageModel.getTextCoordArray());
	mPlaneIndexBuffer = D3D::getInstance().createHomogenousBuffer(mImageModel.getIndexArray(), DXGI_FORMAT_R32G32B32A32_UINT);
	mPlaneTexture = D3D::getInstance().createBasicTexture(mImageModel.getTexture(), mImageModel.getTextureDimension());

	std::vector<LensTransform> scene;
	scene.emplace_back(LensTransform{LensDatabase::get(0), DirectX::XMMatrixIdentity()});
	
	BVHWrapper bvh(scene);
	mBVHBuffer = D3D::getInstance().createStructuredBuffer(bvh.getTree());
	mLensVertexBuffer = D3D::getInstance().createHomogenousBuffer(bvh.getVertices(), DXGI_FORMAT_R32G32B32A32_FLOAT);
	mLensNormalBuffer = D3D::getInstance().createHomogenousBuffer(bvh.getNormals(), DXGI_FORMAT_R32G32B32A32_FLOAT);
	mLensIndexBuffer = D3D::getInstance().createHomogenousBuffer(bvh.getIndices(), DXGI_FORMAT_R32G32B32A32_UINT);
	
	mSampler = D3D::getInstance().createSampler();
	mRenderTexture = D3D::getInstance().createBasicTexture(std::vector<uint32_t>(), {WIDTH, HEIGHT}, DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE);

	ConstantBuffer constants{
		mImageModel.getIndexArray().size() / 3,
		bvh.getIndices().size() / 3
	};
	
	mConstantBuffer.buffer = D3D::getInstance().createBuffer(constants, D3D11_BIND_CONSTANT_BUFFER);
	
	mVertexShader = D3D::getInstance().createShader<uni::VertexShader>(LR"(Assets\Shaders\vertex.hlsl)", "vs_5_0");
	mPixelShader = D3D::getInstance().createShader<uni::PixelShader>(LR"(Assets\Shaders\pixel.hlsl)", "ps_5_0");
	mRaytraceShader = D3D::getInstance().createShader<uni::ComputeShader>(LR"(Assets\Shaders\raytrace.hlsl)", "cs_5_0");
	
	MessageBus::registerListener(this);
}

void Logic::update(float dt)
{
	mCamera.update(dt);
	submitDraw();
}

void Logic::recieveMessage(Message message)
{
}

void Logic::submitDraw()
{
	auto rt = new RayTraceStruct{
		mCamera.getBufferGPU(),
		mBVHBuffer,
		mPlaneVertexBuffer,
		mPlaneTexcoordBuffer,
		mPlaneIndexBuffer,
		mLensVertexBuffer,
		mLensNormalBuffer,
		mLensIndexBuffer,
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

