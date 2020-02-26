#include "D3D.hpp"


void D3D::Init(ID3D11Device* device, ID3D11DeviceContext* context)
{
	mDevice = device;
	mContext = context;
}

D3D& D3D::getInstance()
{
	static D3D instance;
	return instance;
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

uni::ShaderResourceView D3D::createSRV(ID3D11Resource* buffer, DXGI_FORMAT format,
	D3D11_SRV_DIMENSION dimension, size_t numElements, size_t firstElement)
{
	uni::ShaderResourceView resource;
	
	D3D11_SHADER_RESOURCE_VIEW_DESC descriptor = {};
	descriptor.Format = format;
	descriptor.ViewDimension = dimension;

	if (dimension == D3D11_SRV_DIMENSION_BUFFER)
	{
		descriptor.Buffer.FirstElement = firstElement;
		descriptor.Buffer.NumElements = numElements;
	}
	else if (dimension == D3D11_SRV_DIMENSION_TEXTURE2D)
		descriptor.Texture2D.MipLevels = 1;

	mDevice->CreateShaderResourceView(buffer, &descriptor, &resource);
	return resource;
}

uni::UnorderedAccessView D3D::createUAV(ID3D11Resource* buffer, size_t numElements, 
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

	mDevice->CreateUnorderedAccessView(buffer, &descriptor, &resource);
	return resource;
}