#pragma once
#include <d3d11.h>
#include "Util.hpp"

class GraphicWrapper
{
public:
	void Init(ID3D11Device* device);
	static GraphicWrapper& getInstance();

	Buffer createBufferSRV()

private:
	uni::ShaderResourceView createSRV(Buffer& buf);

	uni::UnorderedAccessView createBufferViewUAV(
		ID3D11Buffer& buffer,
		size_t numElements,
		size_t firstElement = 0,
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN
	);
	
private:
	ID3D11Device* mDevice;
	
};