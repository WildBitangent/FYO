#include "Message.hpp"

MessageBus MessageBus::mInstance;

void MessageBus::post(const Message& msg)
{
	mInstance.mBuffer.emplace(msg);
}

void MessageBus::dispatch()
{
	while (!mInstance.mBuffer.empty())
	{
		const auto message = mInstance.mBuffer.pop();
		
		for (auto& listener : mInstance.mListeners)
			listener.recieveMessage(message);
	}
}
