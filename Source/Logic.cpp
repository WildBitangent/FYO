#include "Logic.hpp"
#include "Message.hpp"
#include "Renderer.hpp"


Logic::Logic()
	: mImageModel(R"(Assets/Models/plane.obj)")
	, mLensModel(R"(Assets/Models/lens.obj)")
{
	auto data = std::make_unique<MessageBufferAlloc>();
	data->data = reinterpret_cast<> mImageModel.getVertexArray().data();
	data->size = mImageModel.getVertexArray().size() * sizeof(mImageModel.getVertexArray()[0]);
	
	MessageBus::post({MessageID::REQUEST_ALLOC_BUFFER, )

	
}

void Logic::update(float dt)
{


	
}
