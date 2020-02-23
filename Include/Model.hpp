#pragma once
#include <vector>
#include <DirectXMath.h>
#include <string>


class Model
{
public:
	using Vertices = std::vector<DirectX::XMFLOAT3A>;
	using TextCoord = std::vector<DirectX::XMFLOAT2>;
	using Normals = std::vector<DirectX::XMFLOAT3A>;
	using Indices = std::vector<DirectX::XMUINT4>;
	using Texture = std::vector<uint8_t>;
	
public:
	Model(const std::string& path);

	Vertices& getVertexArray();
	Normals& getNormalArray();
	TextCoord& getTextCoordArray();
	Indices& getIndexArray();
	Texture& getTexture();

	DirectX::XMUINT2 getTextureDimension() const;

private:
	void loadTexture(const std::string& path);
	
private:
	Vertices mVertexArray;
	Normals mNormalArray;
	Indices mIndexArray;
	TextCoord mTextCoordArray;

	Texture mTexture;
	DirectX::XMUINT2 mTextureDimension;
};
