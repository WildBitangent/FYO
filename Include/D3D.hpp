#pragma once
#include <d3d11.h>
#include "Util.hpp"
#include <vector>

namespace DirectX {
	struct XMINT2;
}

class D3D
{
public:
	void Init(ID3D11Device* device);
	static D3D& getInstance();

	template<typename T>
	uni::Buffer createBuffer(
		std::vector<T> data, 
		size_t stride, 
		UINT flags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
		UINT bindFlags = D3D11_BIND_SHADER_RESOURCE,
		UINT cpuFlags = 0,
		D3D11_USAGE usage = D3D11_USAGE_DEFAULT
	);
	
	template<typename T>
	uni::Texure2D createTexture(
		std::vector<T> data, 
		DirectX::XMINT2 resolution,
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM,
		UINT bindFlags = D3D11_BIND_SHADER_RESOURCE,
		UINT cpuFlags = 0,
		UINT flags = 0,
		D3D11_USAGE usage = D3D11_USAGE_DEFAULT
	);
	
	uni::SamplerState createSampler();
	
private:
	uni::ShaderResourceView createSRV(
		ID3D11Buffer& buffer,
		DXGI_FORMAT format,
		D3D11_SRV_DIMENSION dimension, 
		size_t numElements,
		size_t firstElement = 0
	);

	uni::UnorderedAccessView createUAV(
		ID3D11Buffer& buffer,
		size_t numElements,
		D3D11_UAV_DIMENSION dimension, 
		size_t firstElement = 0,
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN
	);
	
private:
	ID3D11Device* mDevice = nullptr;
	
};
