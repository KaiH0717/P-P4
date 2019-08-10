#pragma once

#include <DirectXMath.h>
using namespace DirectX;

class Camera
{
private:
	void UpdateView();
	// storage type
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
	// speed type
	XMVECTOR positionVector;
	XMVECTOR rotationVector;
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;
public:
	Camera();
	~Camera();

	void Reset();

	const XMMATRIX GetViewMatrix() const;
	const XMMATRIX GetProjectionMatrix() const;
	const XMVECTOR GetPositionVector() const;
	const XMVECTOR GetRotationVector() const;

	void SetProjection(float fov, float aspectRatio, float nearPlane, float farPlane);
	void SetPosition(const XMVECTOR& pos);
	void SetPosition(float x, float y, float z);
	void Move(const XMVECTOR& pos);
	void Move(float x, float y, float z);
	void Rotate(const XMVECTOR& rot);
	void Rotate(float x, float y, float z);
};
