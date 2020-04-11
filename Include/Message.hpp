#pragma once
#include "RingBuffer.hpp"
#include <vector>

enum class MessageID : size_t
{
	DRAW_GUI,
	DRAW_RT,
	INPUT_KEY,
	GUI_INIT,
	CAMERA_POS,

	// Module specific messages // TODO maybe not
	LOGIC,
};

struct Message
{
	MessageID messageID;

	union
	{
		int datai;
		float dataf;
		void(*datafun)(void*);
		void* datap;
	};

	int extra = 0;
};

class Listener
{
public:
	virtual void recieveMessage(Message message) = 0;
	virtual Message* recieveExpressMessage(const Message& message) = 0;
};

class MessageBus
{
public:
	static void post(const Message& msg);
	static void dispatch();
	static void registerListener(Listener* listener);
	static void unregister(Listener* listener);
	static Message* postExpress(const Message& msg);
	
private:
	RingBuffer<Message, 50> mBuffer;
	std::vector<Listener*> mListeners;
	
	static MessageBus mInstance;
};


//namespace
//{
//	using Reciever = const char*;
//
//	struct Message_t
//	{
//		Message msg;
//		Reciever rcv;
//	};
//
//	template<typename T>
//	constexpr static const char* getID()
//	{
//#if defined(__GNUC__) || defined(__clang__)
//		return __PRETTY_FUNCTION__;
//#elif defined(_MSC_VER)
//		return __FUNCSIG__;
//#else
//#error unsupported compiler (only GCC, clang and MSVC are supported)
//#endif
//	}
//
//	constexpr static const char* getID()
//	{
//		return nullptr;
//	}
//}
//
//class Listener
//{
//public:
//	virtual void recieveMessage(Message message) = 0;
//	virtual Message* recieveExpressMessage(const Message& message) = 0;
//};
//
//class MessageBus
//{
//public:
//	/// Send message to specific Module
//	template<typename T>
//	static void post(const Message& msg)
//	{
//		mInstance.mBuffer.emplace({ msg, IDManager::get<T>() });
//	}
//
//	/// Send message to every Module
//	static void post(const Message& msg)
//	{
//		mInstance.mBuffer.emplace({ msg, nullptr });
//	}
//
//	static void dispatch();
//	static void registerListener(Listener* listener);
//	static void unregister(Listener* listener);
//	static Message* postExpress(const Message& msg);
//
//private:
//	RingBuffer<Message_t, 50> mBuffer;
//	std::vector<Listener*> mListeners;
//	std::unordered_map<Reciever, Listener> mListenersMap;
//
//	static MessageBus mInstance;
//};