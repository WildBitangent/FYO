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
			listener->recieveMessage(message);
	}
}

void MessageBus::registerListener(Listener* listener)
{
	mInstance.mListeners.emplace_back(listener);
}

void MessageBus::unregister(Listener* listener)
{
	for (size_t i = 0; i < mInstance.mListeners.size(); ++i)
	{
		if (mInstance.mListeners[i] == listener)
		{
			mInstance.mListeners[i] = mInstance.mListeners.back();
			mInstance.mListeners.pop_back();
			break;
		}
	}
}

Message* MessageBus::postExpress(const Message& msg)
{
	for (auto& l : mInstance.mListeners)
		if (auto message = l->recieveExpressMessage(msg); message)
			return message;
}
