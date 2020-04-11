#pragma once
#include <DirectXMath.h>
#include "D3D.hpp"
#include "Message.hpp"


class Camera : public Listener
{
public:
	struct alignas(16) CameraBuffer
	{
		// DirectX::XMVECTOR position = {1.f, 3.f, 348.0f};
		//DirectX::XMVECTOR position = { 1.f, 3.f, 35.0f };
		DirectX::XMVECTOR position = { 0.f, 3.f, 452.5f };
		DirectX::XMVECTOR upperLeftCorner;
		DirectX::XMVECTOR horizontal;
		DirectX::XMVECTOR vertical;
		DirectX::XMFLOAT2 pixelSize;
	};
public:

	Camera();
	
	void updateResolution(size_t width, size_t height);
	void update(float dt);

	void recieveMessage(Message message) override;
	Message* recieveExpressMessage(const Message& message) override;

	CameraBuffer* getBufferCPU();
	Buffer& getBufferGPU();
	DirectX::XMFLOAT3 getDirection();

private:
	CameraBuffer mBufferCPU;
	
	Buffer mBufferGPU;
	
	DirectX::XMVECTOR mFront = {0.f, 0.f, 1.f}; 
	DirectX::XMVECTOR mUp = {0.0f, 1.0f, 0.0f};
	DirectX::XMVECTOR mLeft = {};

	float mHalfWidth;
	float mHalfHeight;

	float mPitch = {}; // todo maybe use radians as base?
	float mYaw = 270;
	// float mYaw = 356;
};
