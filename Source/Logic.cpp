#include "Logic.hpp"
#include "Message.hpp"
#include "D3D.hpp"


Logic::Logic()
	: mImageModel(R"(Assets/Models/plane.obj)")
	, mLensModel(R"(Assets/Models/lens.obj)")
{
	auto imageVertices = D3D::getInstance().createBuffer(
		mImageModel.getVertexArray(),
		sizeof(mImageModel.getVertexArray()[0])
	);

	auto texture = D3D::getInstance().createTexture(
		mImageModel.getTextureArray(), 
		mImageModel.getTextureDimension()	
	);

	auto sampler = D3D::getInstance().createSampler();

	auto lensVertices = D3D::getInstance().createBuffer(
		mLensModel.getVertexArray(),
		sizeof(mLensModel.getVertexArray()[0])
	);
}

void Logic::update(float dt)
{


	
}
