#include "Model.hpp"
#include "tinyobjloader/tiny_obj_loader.h"
#include "lodepng/lodepng.h"
#include "Message.hpp"

Model::Model(const std::string& path)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	// todo some error checks
	bool ret = LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
	
	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++)
	{
		// Loop over faces(polygon)
		size_t indexOffset = 0;

		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
		{
			size_t fv = shapes[s].mesh.num_face_vertices[f];

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++)
			{
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[indexOffset + v];

				mVertexArray.emplace_back(attrib.vertices.data() + 3 * idx.vertex_index);
				mTextCoordArray.emplace_back(attrib.texcoords.data() + 2 * idx.texcoord_index);
				// mNormalArray.emplace_back(3 * idx.normal_index);
			}
			indexOffset += fv;
		}
	}

	if (!materials[0].diffuse_texname.empty())
		loadTexture(materials[0].diffuse_texname);
}

Model::Vertices& Model::getVertexArray()
{
	return mVertexArray;
}

Model::TextCoord& Model::getTextCoordArray()
{
	return mTextCoordArray;
}

Model::Texture& Model::getTextureArray()
{
	return mTexture;
}

void Model::loadTexture(const std::string& path)
{
	// todo need to prepend path
	unsigned error = lodepng::decode(mTexture, mTextureDimension.x, mTextureDimension.y, path);

	// todo error check
	// std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
}
