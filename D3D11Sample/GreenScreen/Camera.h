#pragma once

#include <DirectXMath.h>
using namespace DirectX;

class Camera
{
private:
	void UpdateViewMatrix();
	// storage type
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	// speed type
	XMVECTOR positionVector;
	XMVECTOR rotationVector;
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;

	const XMVECTOR DEFAULT_FORWARD_VECTOR = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	const XMVECTOR DEFAULT_UP_VECTOR = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
public:
	Camera();
	void SetProjectionValues(float fovDegrees, float aspectRatio, float nearZ, float farZ);

	const XMMATRIX& GetViewMatrix() const;
	const XMMATRIX& GetProjectionMatrix() const;

	const XMVECTOR& GetPositionVector() const;
	const XMFLOAT3& GetPositionFloat3() const;
	const XMVECTOR& GetRotationVector() const;
	const XMFLOAT3& GetRotationFloat3() const;

	void SetPosition(const XMVECTOR& pos);
	void SetPosition(float x, float y, float z);
	void Move(const XMVECTOR& pos);
	void Move(float x, float y, float z);
	void Rotate(const XMVECTOR& rot);
	void Rotate(float x, float y, float z);
};
