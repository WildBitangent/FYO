#pragma once
#include "RingBuffer.hpp"
#include <vector>

enum class MessageID : size_t
{
	DRAW,
	DRAW_RT,
	INPUT_MONITOR_KEY,
	UPDATE_CAMERA,
};


struct Message
{
	MessageID messageID;
	union
	{
		float dataf;
		int datai;
		void(*datafun)(void*);
		void* datap;
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
	static void registerListener(Listener* listener);
	static void unregister(Listener* listener);
	
private:
	RingBuffer<Message, 50> mBuffer;
	std::vector<Listener*> mListeners;
	
	static MessageBus mInstance;
};