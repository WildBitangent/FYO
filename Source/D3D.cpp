#include "D3D.hpp"
#include "DirectXMath.h"

void D3D::Init(ID3D11Device* device)
{
	mDevice = device;
}

D3D& D3D::getInstance()
{
	static D3D instance;
	return instance;
}

template <typename T>
uni::Buffer D3D::createBuffer(std::vector<T> data, size_t stride, UINT flags, UINT bindFlags, UINT cpuFlags, D3D11_USAGE usage)
{
	uni::Buffer buffer;

	D3D11_BUFFER_DESC descriptor = {};
	descriptor.Usage = usage;
	descriptor.StructureByteStride = stride;
	descriptor.ByteWidth = stride * data.size();
	descriptor.BindFlags = bindFlags;
	descriptor.CPUAccessFlags = cpuFlags;
	descriptor.MiscFlags = flags;

	D3D11_SUBRESOURCE_DATA bufferData = {};
	bufferData.pSysMem = data.data();

	mDevice->CreateBuffer(&descriptor, &bufferData, &buffer);
	return buffer;	
}

template <typename T>
uni::Texure2D D3D::createTexture(std::vector<T> data, DirectX::XMINT2 resolution, 
	DXGI_FORMAT format, UINT bindFlags, UINT cpuFlags, UINT flags, D3D11_USAGE usage)
{
	uni::Texure2D texture;
	
	D3D11_TEXTURE2D_DESC descriptor = {};
	descriptor.Width = resolution.x;
	descriptor.Height = resolution.y;
	descriptor.MipLevels = 1;
	descriptor.ArraySize = 1;
	descriptor.Format = format;
	descriptor.SampleDesc.Count = 1;
	descriptor.SampleDesc.Quality = 0;
	descriptor.Usage = usage;
	descriptor.BindFlags = bindFlags;
	descriptor.CPUAccessFlags = cpuFlags;
	descriptor.MiscFlags = flags;

	D3D11_SUBRESOURCE_DATA textureData;
	textureData.pSysMem = data.data();
	// textureData.SysMemPitch = dimension * 4;
	// textureData.SysMemSlicePitch = textureSize;

	mDevice->CreateTexture2D(&descriptor, &textureData, &texture);
	return texture;
}

uni::SamplerState D3D::createSampler()
{
	uni::SamplerState sampler;
	
	D3D11_SAMPLER_DESC descriptor = {};
	descriptor.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	descriptor.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	descriptor.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	descriptor.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	descriptor.ComparisonFunc = D3D11_COMPARISON_NEVER;
	descriptor.MinLOD = 0;
	descriptor.MaxLOD = D3D11_FLOAT32_MAX;

	mDevice->CreateSamplerState(&descriptor, &sampler);
	return sampler;
}

uni::ShaderResourceView D3D::createSRV(ID3D11Buffer& buffer, DXGI_FORMAT format,
	D3D11_SRV_DIMENSION dimension, size_t numElements, size_t firstElement)
{
	uni::ShaderResourceView resource;
	
	D3D11_SHADER_RESOURCE_VIEW_DESC descriptor = {};
	descriptor.Format = format;
	descriptor.ViewDimension = dimension;

	if (dimension == D3D10_1_SRV_DIMENSION_BUFFER)
	{
		descriptor.Buffer.FirstElement = firstElement;
		descriptor.Buffer.NumElements = numElements;
	}
	else if (dimension == D3D11_SRV_DIMENSION_TEXTURE2D)
		descriptor.Texture2D.MipLevels = 1;

	mDevice->CreateShaderResourceView(&buffer, &descriptor, &resource);
	return resource;
}

uni::UnorderedAccessView D3D::createUAV(ID3D11Buffer& buffer, size_t numElements, 
	D3D11_UAV_DIMENSION dimension, size_t firstElement, DXGI_FORMAT format)
{
	uni::UnorderedAccessView resource;

	D3D11_UNORDERED_ACCESS_VIEW_DESC descriptor = {};
	descriptor.Format = format;
	descriptor.ViewDimension = dimension;

	if (dimension == D3D11_UAV_DIMENSION_BUFFER)
	{
		descriptor.Buffer.FirstElement = firstElement;
		descriptor.Buffer.NumElements = numElements;
	}

	mDevice->CreateUnorderedAccessView(&buffer, &descriptor, &resource);
	return resource;
}
