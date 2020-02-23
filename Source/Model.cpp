#include "Model.hpp"
#include "tinyobjloader/tiny_obj_loader.h"
#include "lodepng/lodepng.h"
#include "Message.hpp"

Model::Model(const std::string& path) // todo paths
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	// todo some error checks
	bool ret = LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()); 

	// align vertices
	for (size_t i = 0; i < attrib.vertices.size(); i += 3)
		mVertexArray.emplace_back(DirectX::XMFLOAT3A(attrib.vertices.data() + i));

	for (size_t i = 0; i < attrib.normals.size(); i += 3)
		mNormalArray.emplace_back(DirectX::XMFLOAT3A(attrib.normals.data() + i));

	for (size_t i = 0; i < attrib.texcoords.size(); i += 2)
		mTextCoordArray.emplace_back(DirectX::XMFLOAT2(attrib.texcoords.data() + i));
	
	// There is gonna be only 1 shape in each model and only triangular faces
	std::vector<uint32_t> indices[3];
	for (auto& idx : shapes[0].mesh.indices)
	{
		indices[0].emplace_back(idx.vertex_index);
		indices[1].emplace_back(idx.normal_index);
		indices[2].emplace_back(idx.texcoord_index);
	}

	// concatenate indices
	for (auto& c : indices)
		for (size_t i = 0; i < c.size(); i += 3)
			mIndexArray.emplace_back(c[i], c[i + 1], c[i + 2], 0);

	if (!materials.empty() && !materials[0].diffuse_texname.empty())
		loadTexture(materials[0].diffuse_texname);
}

Model::Vertices& Model::getVertexArray()
{
	return mVertexArray;
}

Model::Normals& Model::getNormalArray()
{
	return mNormalArray;
}

Model::TextCoord& Model::getTextCoordArray()
{
	return mTextCoordArray;
}

Model::Indices& Model::getIndexArray()
{
	return mIndexArray;
}

Model::Texture& Model::getTexture()
{
	return mTexture;
}

DirectX::XMUINT2 Model::getTextureDimension() const
{
	return mTextureDimension;
}

void Model::loadTexture(const std::string& path)
{
	// todo need to prepend path
	unsigned error = lodepng::decode(mTexture, mTextureDimension.x, mTextureDimension.y, path);

	// todo error check
	// std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
}
