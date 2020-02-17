#pragma once
#include "RingBuffer.hpp"
#include <memory>

enum class MessageID : size_t
{
	DRAW,
	INPUT_MONITOR_KEY,
	REQUEST_ALLOC_BUFFER,
};


struct Message
{
	MessageID messageID;
	union
	{
		float dataf;
		int datai;
		std::unique_ptr<void*> datap;
		void(*datafun)(void*);
	};
};

class Listener
{
public:
	virtual void recieveMessage(Message message) = 0;
};

class MessageBus
{
public:
	static void post(const Message& msg);
	static void dispatch();
	
private:
	RingBuffer<Message, 50> mBuffer;
	std::vector<Listener> mListeners;
	
	static MessageBus mInstance;
};