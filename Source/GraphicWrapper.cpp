#include "GraphicWrapper.hpp"

void GraphicWrapper::Init(ID3D11Device* device)
	: mDevice(device)
{
	
}

GraphicWrapper& GraphicWrapper::getInstance()
{
	static GraphicWrapper instance;
	return instance;
}

Buffer GraphicWrapper::createBuffer()
{
	Buffer buffer;
	
	D3D11_BUFFER_DESC pathStateDescriptor = {};
	pathStateDescriptor.Usage = D3D11_USAGE_DEFAULT;
	pathStateDescriptor.ByteWidth = PATHCOUNT * 200;
	pathStateDescriptor.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	pathStateDescriptor.CPUAccessFlags = 0;
	pathStateDescriptor.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	pathStateDescriptor.StructureByteStride = 200;

	
	mDevice->CreateBuffer(&pathStateDescriptor, nullptr, &mPathStateBuffer);

	
}

uni::ShaderResourceView GraphicWrapper::createBufferViewUAV(ID3D11Buffer& buffer, size_t numElements, size_t firstElement, DXGI_FORMAT format)
{
	uni::UnorderedAccessView resourceUAV;
	
	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDescriptor = {};
	UAVDescriptor.Format = format;
	UAVDescriptor.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	UAVDescriptor.Buffer.FirstElement = firstElement;
	UAVDescriptor.Buffer.NumElements = numElements;

	mDevice->CreateUnorderedAccessView(&buffer, &UAVDescriptor, &resourceUAV);
}
