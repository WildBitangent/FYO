#include "DirectXMath.h"
#include <d3dcompiler.h>
#include "fmt.h"

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
uni::Buffer D3D::createBuffer(T& data, UINT bindFlags, UINT flags, UINT cpuFlags, D3D11_USAGE usage)
{
	uni::Buffer buffer;

	D3D11_BUFFER_DESC descriptor = {};
	descriptor.Usage = usage;
	descriptor.ByteWidth = sizeof(T);
	descriptor.BindFlags = bindFlags;
	descriptor.CPUAccessFlags = cpuFlags;
	descriptor.MiscFlags = flags;

	D3D11_SUBRESOURCE_DATA bufferData = {};
	bufferData.pSysMem = &data;

	mDevice->CreateBuffer(&descriptor, &bufferData, &buffer);
	return buffer;	
}

template <typename T>
Buffer D3D::createStructuredBuffer(std::vector<T> data, UINT bindFlags, UINT cpuFlags, D3D11_USAGE usage)
{
	Buffer buffer;
	buffer.buffer = createBuffer(data, sizeof(T), D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, bindFlags, cpuFlags, usage);

	if (bindFlags & D3D11_BIND_SHADER_RESOURCE)
		buffer.srv = createSRV(buffer.buffer, DXGI_FORMAT_UNKNOWN, D3D11_SRV_DIMENSION_BUFFER, data.size(), 0);
	if (bindFlags & D3D11_BIND_UNORDERED_ACCESS)
		buffer.uav = createUAV(buffer.buffer, data.size(), D3D11_UAV_DIMENSION_BUFFER, 0, DXGI_FORMAT_UNKNOWN);

	return buffer;
}

template <typename T>
Buffer D3D::createHomogenousBuffer(std::vector<T> data, DXGI_FORMAT format, UINT bindFlags, UINT flags, UINT cpuFlags, D3D11_USAGE usage)
{
	Buffer buffer;
	buffer.buffer = createBuffer(data, sizeof(T), flags, bindFlags, cpuFlags, usage);

	if (bindFlags & D3D11_BIND_SHADER_RESOURCE)
		buffer.srv = createSRV(buffer.buffer, format, D3D11_SRV_DIMENSION_BUFFER, data.size(), 0);
	if (bindFlags & D3D11_BIND_UNORDERED_ACCESS)
		buffer.uav = createUAV(buffer.buffer, data.size(), D3D11_UAV_DIMENSION_BUFFER, 0, DXGI_FORMAT_UNKNOWN);

	return buffer;
}

template <typename T>
uni::Texure2D D3D::createTexture(std::vector<T> data, DirectX::XMUINT2 resolution, 
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

	D3D11_SUBRESOURCE_DATA textureData = {};
	textureData.pSysMem = data.data();
	textureData.SysMemPitch = resolution.x * 4;

	auto subresource = data.empty() ? nullptr : &textureData;
	
	mDevice->CreateTexture2D(&descriptor, subresource, &texture);
	return texture;
}

template <typename T>
Texture D3D::createBasicTexture(std::vector<T> data, DirectX::XMUINT2 resolution, DXGI_FORMAT format, UINT bindFlags,
	UINT cpuFlags, UINT flags, D3D11_USAGE usage)
{
	Texture texture;
	texture.texture = createTexture(data, resolution, format, bindFlags, cpuFlags, flags, usage);
	texture.resolution = resolution;

	if (bindFlags & D3D11_BIND_SHADER_RESOURCE)
		texture.srv = createSRV(texture.texture, format, D3D11_SRV_DIMENSION_TEXTURE2D, 0);
	if (bindFlags & D3D11_BIND_UNORDERED_ACCESS)
		texture.uav = createUAV(texture.texture, 0, D3D11_UAV_DIMENSION_TEXTURE2D, 0, format);

	return texture;
}

template <typename T>
T D3D::createShader(const std::wstring& path, const std::string& target)
{
	T shader;
	uni::Blob err;
	uni::Blob compiled;
	
	auto result = D3DCompileFromFile(path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", target.c_str(), {}, {}, &compiled, &err);
	if (result != S_OK)
		throw std::runtime_error(fmt::format("Failed to compile {}. ERR: {}\n\n{}", std::string(path.begin(), path.end()), result, reinterpret_cast<const char*>(err->GetBufferPointer())));

	if constexpr (std::is_same_v<T, uni::VertexShader>)
		result = mDevice->CreateVertexShader(compiled->GetBufferPointer(), compiled->GetBufferSize(), nullptr, &shader);
	else if constexpr (std::is_same_v<T, uni::PixelShader>)
		result = mDevice->CreatePixelShader(compiled->GetBufferPointer(), compiled->GetBufferSize(), nullptr, &shader);
	else if constexpr (std::is_same_v<T, uni::ComputeShader>)
		result = mDevice->CreateComputeShader(compiled->GetBufferPointer(), compiled->GetBufferSize(), nullptr, &shader);
	else
		static_assert("Unsupported shader.");

	if (result != S_OK)
		throw std::runtime_error("Failed to create shader. ");
	
	return std::move(shader);
}
