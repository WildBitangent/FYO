#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <utility>

#include "UniqueDX11.hpp"
#include "Camera.hpp"
#include "Util.hpp"
#include "Message.hpp"
#include <string>


class Renderer /*: public Listener*/
{
	using Resolution = DirectX::XMUINT2;
public:
	Renderer(Renderer&) = delete;
	Renderer(Renderer&&) = delete;
	Renderer& operator=(Renderer&) = delete;
	Renderer& operator=(Renderer&&) = delete;

	// virtual ~Renderer() = default;
	// void recieveMessage(Message message) override;
	void init(HWND hwnd, Resolution resolution);

	template<typename T>
	static T createShader(const std::wstring& path, const std::string& target);

	template<typename T>
	static Buffer createBuffer(std::vector<T> data, D3D11_BUFFER_DESC descriptor);

	
	void update(float dt);
	void draw();



private:
	void initScene();
	
	void reloadShader(); // TODO rewrite

private:
	// Camera mCamera;
	// ModelManager mModel;
	// uint32_t mNumIndices;
	

	uni::Swapchain mSwapChain;
	uni::Device mDevice;
	uni::DeviceContext mContext;
	uni::RenderTargetView mRenderTarget;
	
	// uni::VertexShader mVertexShader;
	// uni::PixelShader mPixelShader;
	//
	// uni::ComputeShader mShaderLogic;
	// uni::ComputeShader mShaderNewPath;
	// uni::ComputeShader mShaderMaterialUE4;
	// uni::ComputeShader mShaderMaterialGlass;
	// uni::ComputeShader mShaderExtensionRay;
	// uni::ComputeShader mShaderShadowRay;
	
	uni::InputLayout mVertexLayout;

	uni::Texure2D mRenderTexture;
	uni::ShaderResourceView mRenderTextureSRV;
	uni::UnorderedAccessView mRenderTextureUAV;
	
	// Buffer mLightBuffer;
	//
	// uni::Buffer mCameraBuffer;
	// uni::Buffer mPathStateBuffer;
	// uni::Buffer mQueueBuffer;
	// uni::Buffer mQueueCountersBuffer;

	// uni::UnorderedAccessView mPathStateUAV;
	// uni::UnorderedAccessView mQueueUAV;
	// uni::UnorderedAccessView mQueueCountersUAV;

	// uni::SamplerState mSampler;
	// uni::ShaderResourceView mTexture;
	//
	// friend class GUI;

	static Renderer mInstance;
};
