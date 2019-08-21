#pragma once

#include <DirectXMath.h>
using namespace DirectX;

class Camera
{
private:
	// attributes
	float FOV;
	float aspectRatio;
	float nearPlane;
	float farPlane;

	XMFLOAT4X4 worldView;
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
public:
	Camera();

	////////////////////////////////////////
	// projection matrix getters
	////////////////////////////////////////
	float GetFOV() const;
	float GetAspectRatio() const;
	float GetNearPlane() const;
	float GetFarPlane() const;
	const XMMATRIX GetViewMatrix() const;
	const XMMATRIX GetProjectionMatrix() const;
	const XMFLOAT4 GetWorldPosition() const;
	const XMFLOAT4 GetLocalPosition() const;

	////////////////////////////////////////
	// projection matrix setters
	////////////////////////////////////////
	void SetFOV(float fov);
	void SetAspectRatio(float aspectRatio);
	void SetNearPlane(float nearPlane);
	void SetFarPlane(float farPlane);
	void SetProjection(float fov, float aspectRatio, float nearPlane, float farPlane);

	////////////////////////////////////////
	// view matrix setters
	////////////////////////////////////////
	void SetPosition(float x, float y, float z);

	////////////////////////////////////////
	// projection matrix functions
	////////////////////////////////////////
	void IncreaseFOV(float offset);
	void DecreaseFOV(float offset);
	void IncreaseNearPlane(float offset);
	void DecreaseNearPlane(float offset);
	void IncreaseFarPlane(float offset);
	void DecreaseFarPlane(float offset);
	void UpdateView();

	////////////////////////////////////////
	// view matrix functions
	////////////////////////////////////////
	void MoveX(float offset);
	void MoveY(float offset);
	void MoveZ(float offset);
	void Pitch(float degrees);
	void Yaw(float degrees);
	void Roll(float degrees);
	//void LookAt(const XMVECTOR& position);
};
