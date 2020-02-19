#include "Logic.hpp"
#include "Message.hpp"
#include "D3D.hpp"


Logic::Logic()
	: mImageModel(R"(Assets/Models/plane.obj)")
	, mLensModel(R"(Assets/Models/lens.obj)")
{
	auto data = std::make_unique<MessageBufferAlloc>();
	data->data = reinterpret_cast<> mImageModel.getVertexArray().data();
	data->size = mImageModel.getVertexArray().size() * sizeof(mImageModel.getVertexArray()[0]);
	
	D3D::getInstance().createBuffer(mImageModel.)

	
}

void Logic::update(float dt)
{


	
}
