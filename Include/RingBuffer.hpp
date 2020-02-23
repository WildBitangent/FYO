#pragma once
#include <array>
#include <cassert>

template<typename A, int N>
class RingBuffer
{
public:
	A& back()
	{
		assert(mHead == mTail);
		return mBuffer[mTail];
	}

	A pop()
	{
		assert(mHead != mTail);

		auto element = mBuffer[mTail];
		mTail = ++mTail % N;
		return element;
		
	}

	void emplace(const A& element)
	{
		mBuffer[mHead] = element;
		mHead = ++mHead % N;
	}

	bool empty()
	{
		return mHead == mTail;
	}

private:
	std::array<A, N> mBuffer;
	size_t mHead = 0;
	size_t mTail = 0;
};
