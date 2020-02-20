#pragma once
#include <vector>
#include <DirectXMath.h>
#include <string>


class Model
{
public:
	using Vertices = std::vector<DirectX::XMFLOAT3A>;
	using TextCoord = std::vector<DirectX::XMFLOAT2>;
	using Texture = std::vector<uint8_t>;
	
public:
	Model(const std::string& path);

	Vertices& getVertexArray();
	TextCoord& getTextCoordArray();
	Texture& getTextureArray();

	DirectX::XMUINT2 getTextureDimension() const;

private:
	void loadTexture(const std::string& path);
	
private:
	Vertices mVertexArray;
	// std::vector<uint32_t> mIndexArray;
	TextCoord mTextCoordArray;
	// std::vector<DirectX::XMFLOAT3A> mNormalArray;

	Texture mTexture;
	DirectX::XMUINT2 mTextureDimension;
};
