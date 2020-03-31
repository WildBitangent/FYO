#include "Camera.hpp"
#include "Constants.hpp"
#include "windows.h"
#include "Input.hpp"

using namespace DirectX;

Camera::Camera()
{
	updateResolution(WIDTH, HEIGHT);

	mBufferGPU.buffer = D3D::getInstance().createBuffer(mBufferCPU, D3D11_BIND_CONSTANT_BUFFER);
}

void Camera::updateResolution(size_t width, size_t height)
{
	const auto theta = 60 * 3.14f / 180; // 60
    const auto aspect = width / static_cast<float>(height);

    mHalfHeight = tanf(theta / 2.f);
    mHalfWidth = aspect * mHalfHeight;

	mBufferCPU.pixelSize = XMFLOAT2(1.f / width, 1.f / height);
}

void Camera::update(float dt)
{	
	// mouse handling
	const auto delta = Input::getInstance().getMouseDelta();
	mYaw += delta.x;
	mPitch -= delta.y;
	
    if (mPitch > 89.0f) mPitch = 89.0f;
    if (mPitch < -89.0f) mPitch = -89.0f;

	mFront = {
		cosf(XMConvertToRadians(mYaw)) * cosf(XMConvertToRadians(mPitch)),
		sinf(XMConvertToRadians(mPitch)),
		sinf(XMConvertToRadians(mYaw)) * cosf(XMConvertToRadians(mPitch)),
	};

	mFront = XMVector3Normalize(mFront);
	mLeft = XMVector3Normalize(XMVector3Cross({0, 1, 0}, mFront));
	mUp = XMVector3Normalize(XMVector3Cross(mFront, mLeft));
	
	// keyboard handling
	auto speed = 5.f;
	if (Input::getInstance().keyActive(VK_SHIFT))
		speed /= 3;
	if (Input::getInstance().keyActive(VK_CONTROL))
		speed *= 2;

	const auto velocity = speed * dt;
    if (Input::getInstance().keyActive('W'))
        mBufferCPU.position += mFront * velocity;
    if (Input::getInstance().keyActive('S'))
        mBufferCPU.position -= mFront * velocity;
    if (Input::getInstance().keyActive('A'))
        mBufferCPU.position += mLeft * velocity;
    if (Input::getInstance().keyActive('D'))
        mBufferCPU.position -= mLeft * velocity;
	

	// get view matrix and make uniforms
	auto view = XMMatrixTranspose(XMMatrixLookAtRH(mBufferCPU.position, mFront + mBufferCPU.position, mUp));
	
    const auto left = view.r[0];
    const auto up = view.r[1];
    const auto w = view.r[2];

	mBufferCPU.upperLeftCorner = -mHalfWidth * left + mHalfHeight * up - w;
	mBufferCPU.horizontal = 2 * mHalfWidth * left;
	mBufferCPU.vertical = 2 * mHalfHeight * up;
	
	D3D::getInstance().updateBuffer(mBufferGPU.buffer, &mBufferCPU);
}

void Camera::recieveMessage(Message message)
{
}

Message* Camera::recieveExpressMessage(const Message& message)
{
	return nullptr;
}

Camera::CameraBuffer* Camera::getBufferCPU()
{
	return &mBufferCPU;
}

Buffer& Camera::getBufferGPU()
{
	return mBufferGPU;
}

DirectX::XMFLOAT3 Camera::getDirection()
{
	XMFLOAT3 data;
	XMStoreFloat3(&data, mFront);
	return data;
}
