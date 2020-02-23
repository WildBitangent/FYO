#pragma once
#include <d3d11.h>
#include <vector>
#include <string>
#include <DirectXMath.h>
#include "UniqueDX11.hpp"


struct Buffer
{
	uni::Buffer buffer;
	uni::ShaderResourceView srv;
	uni::UnorderedAccessView uav;
};

struct Texture
{
	uni::Texure2D texture;
	uni::ShaderResourceView srv;
	uni::UnorderedAccessView uav;

	DirectX::XMUINT2 resolution;
}; 


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
	uni::Buffer createBuffer(
		T& data, 
		UINT bindFlags = D3D11_BIND_SHADER_RESOURCE,
		UINT flags = 0,
		UINT cpuFlags = 0,
		D3D11_USAGE usage = D3D11_USAGE_DEFAULT
	);

	template<typename T>
	Buffer createStructuredBuffer(
		std::vector<T> data,
		UINT bindFlags = D3D11_BIND_SHADER_RESOURCE,
		UINT cpuFlags = 0,
		D3D11_USAGE usage = D3D11_USAGE_DEFAULT
	);

	template<typename T>
	Buffer createHomogenousBuffer(
		std::vector<T> data,
		DXGI_FORMAT format = DXGI_FORMAT_R32_UINT,
		UINT bindFlags = D3D11_BIND_SHADER_RESOURCE,
		UINT flags = 0,
		UINT cpuFlags = 0,
		D3D11_USAGE usage = D3D11_USAGE_DEFAULT
	);
	
	template<typename T>
	uni::Texure2D createTexture(
		std::vector<T> data, 
		DirectX::XMUINT2 resolution,
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM,
		UINT bindFlags = D3D11_BIND_SHADER_RESOURCE,
		UINT cpuFlags = 0,
		UINT flags = 0,
		D3D11_USAGE usage = D3D11_USAGE_DEFAULT
	);

	template<typename T>
	Texture createBasicTexture(
		std::vector<T> data, 
		DirectX::XMUINT2 resolution,
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM,
		UINT bindFlags = D3D11_BIND_SHADER_RESOURCE,
		UINT cpuFlags = 0,
		UINT flags = 0,
		D3D11_USAGE usage = D3D11_USAGE_DEFAULT
	);
	
	uni::SamplerState createSampler();
	
	uni::ShaderResourceView createSRV(
		ID3D11Resource* buffer,
		DXGI_FORMAT format,
		D3D11_SRV_DIMENSION dimension, 
		size_t numElements,
		size_t firstElement = 0
	);

	uni::UnorderedAccessView createUAV(
		ID3D11Resource* buffer,
		size_t numElements,
		D3D11_UAV_DIMENSION dimension, 
		size_t firstElement = 0,
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN
	);

	template<typename T>
	T createShader(const std::wstring& path, const std::string& target);

	
private:
	ID3D11Device* mDevice = nullptr;
	
};

#include "D3D.inl"