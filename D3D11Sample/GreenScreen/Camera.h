#pragma once

#include <DirectXMath.h>
using namespace DirectX;

class Camera
{
private:
	void UpdateView();
	// attributes
	float FOV;
	float aspectRatio;
	float nearPlane;
	float farPlane;
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

	float GetFOV() const;
	float GetAspectRatio() const;
	float GetNearPlane() const;
	float GetFarPlane() const;
	const XMMATRIX GetViewMatrix() const;
	const XMMATRIX GetProjectionMatrix() const;
	const XMVECTOR GetPositionVector() const;
	const XMVECTOR GetRotationVector() const;

	void SetFOV(float fov);
	void SetAspectRatio(float aspectRatio);
	void SetNearPlane(float nearPlane);
	void SetFarPlane(float farPlane);
	void SetProjection(float fov, float aspectRatio, float nearPlane, float farPlane);
	void SetPosition(const XMVECTOR& position);
	void SetPosition(float x, float y, float z);
	void Move(const XMVECTOR& position);
	void Move(float x, float y, float z);
	void Rotate(const XMVECTOR& rotation);
	void Rotate(float x, float y, float z);

	void IncreaseFOV(float offset);
	void DecreaseFOV(float offset);
	void IncreaseNearPlane(float offset);
	void DecreaseNearPlane(float offset);
	void IncreaseFarPlane(float offset);
	void DecreaseFarPlane(float offset);
};
