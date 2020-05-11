#define FMT_HEADER_ONLY

#include "Renderer.hpp"
#include "Constants.hpp"
#include "fmt.h"
#include "GUI.hpp"

#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <stdexcept>
#include <vector>
#include <array>
#include "Input.hpp"
#include <thread>

using namespace DirectX;

extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

void Renderer::init(HWND hwnd, Resolution resolution)
{
	DXGI_MODE_DESC bufferDesc = {};
	bufferDesc.Width = resolution.x;
	bufferDesc.Height = resolution.y;
	bufferDesc.RefreshRate.Numerator = 60;
	bufferDesc.RefreshRate.Denominator = 1;
	bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferDesc = bufferDesc;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	//IDXGIFactory2* pFactory;
	//CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, __uuidof(IDXGIFactory), reinterpret_cast<void**>(&pFactory));
	//auto factory6 = reinterpret_cast<IDXGIFactory6*>(pFactory);

	//IDXGIAdapter* adapter;
	//factory6->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
	//pFactory->Release();

	if (const auto result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0/*D3D11_CREATE_DEVICE_DEBUG*/, nullptr, 0,
		D3D11_SDK_VERSION, &swapChainDesc, &mSwapChain, &mDevice, nullptr, &mContext); result != S_OK)
		throw std::runtime_error(fmt::format("Failed to create device with Swapchain. ERR: {}", result));

	ID3D11Texture2D* backBuffer;
	if (const auto result = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)); result != S_OK)
		throw std::runtime_error(fmt::format("Failed to create Back Buffer. ERR: {}", result));

	if (const auto result = mDevice->CreateRenderTargetView(backBuffer, nullptr, &mRenderTarget); result != S_OK)
		throw std::runtime_error(fmt::format("Failed to create Render Target View. ERR: {}", result));

	mContext->OMSetRenderTargets(1, &mRenderTarget, nullptr);
	backBuffer->Release();	
	
	mContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = WIDTH;
	viewport.Height = HEIGHT;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	mContext->RSSetViewports(1, &viewport);

	mHwnd = hwnd;

	D3D::getInstance().Init(mDevice, mContext);
	MessageBus::registerListener(this);
}

Renderer::~Renderer()
{
	mContext->ClearState();
	mContext->Flush();
}

void Renderer::recieveMessage(Message message)
{
	if (message.messageID == MessageID::DRAW_GUI)
	{
		reinterpret_cast<LensGUI*>(message.datap)->render();
	}
	else if (message.messageID == MessageID::DRAW_RT)
	{
		auto data = reinterpret_cast<RayTraceStruct*>(message.datap);
		draw(*data);
		delete data;
	}
}

Message* Renderer::recieveExpressMessage(const Message& message)
{
	if (message.messageID == MessageID::GUI_INIT)
	{
		reinterpret_cast<LensGUI*>(message.datap)->init(mHwnd, mDevice, mContext);
		return const_cast<Message*>(&message); // just to stop the sender recursion
	}

	return nullptr;
}

void Renderer::update(float dt)
{
	// if (Input::getInstance().keyActive('R'))
	// {
	// 	reloadShader();
	// 	mScene.mCamera.getBuffer()->iterationCounter = 0;
	// }
	//
	// mContext->UpdateSubresource(mCameraBuffer, 0, nullptr, mScene.mCamera.getBuffer(), 0, 0);
}

void Renderer::draw(RayTraceStruct& rayStruct)
{
	std::vector<ID3D11Buffer*> uniforms = {
		rayStruct.camera.buffer,
		rayStruct.constantBuffer.buffer
	};
	
	std::vector<ID3D11ShaderResourceView*> SRVs = {
		rayStruct.lensArray.srv,
		rayStruct.planeVertexBuffer.srv,
		rayStruct.planeTexcoordBuffer.srv,
		rayStruct.planeIndexBuffer.srv,
		rayStruct.planeTexture.srv,
		rayStruct.raysArray.srv
	};
	
	std::vector<ID3D11ShaderResourceView*> nullSRV(SRVs.size());
	std::vector<ID3D11UnorderedAccessView*> nullUAV(1);

	// set shaders
	mContext->VSSetShader(rayStruct.vertexShader, nullptr, 0);
	mContext->PSSetShader(rayStruct.pixelShader, nullptr, 0);
	mContext->CSSetShader(rayStruct.raytraceShader, nullptr, 0);

	// bind buffers for Raytracing
	mContext->CSSetConstantBuffers(0, uniforms.size(), uniforms.data());
	mContext->CSSetShaderResources(0, SRVs.size(), SRVs.data());
	mContext->CSSetUnorderedAccessViews(0, 1, &rayStruct.renderTexture.uav, nullptr);
	mContext->CSSetSamplers(0, 1, &rayStruct.sampler);

	// dispatch
	mContext->Dispatch(WIDTH / 32, HEIGHT / 8, 1);

	// null bound buffers
	mContext->CSSetShaderResources(0, nullSRV.size(), nullSRV.data());
	mContext->CSSetUnorderedAccessViews(0, nullUAV.size(), nullUAV.data(), nullptr);

	// prepare for drawing
	mContext->ClearRenderTargetView(mRenderTarget, std::array<float, 4>({ 0, 0, 0, 0.0f }).data());

	// set buffers for drawing
	mContext->PSSetSamplers(0, 1, &rayStruct.sampler);
	mContext->PSSetShaderResources(0, 1, &rayStruct.renderTexture.srv);

	// draw 
	mContext->Draw(4, 0);

	// null bound buffers
	mContext->PSSetShaderResources(0, 1, nullSRV.data());
}


void Renderer::present()
{
	mSwapChain->Present(0, 0);
}
