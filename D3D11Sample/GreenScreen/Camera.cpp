#include "Camera.h"

Camera::Camera()
{
	FOV = 90.0f;
	aspectRatio = 1.0f;
	nearPlane = 0.1f;
	farPlane = 10.0f;

	viewMatrix = XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	XMStoreFloat4x4(&view, viewMatrix);
	worldViewMatrix = XMMatrixInverse(nullptr, viewMatrix);
	XMStoreFloat4x4(&worldView, worldViewMatrix);

	projectionMatrix = XMMatrixIdentity();
	XMStoreFloat4x4(&projection, projectionMatrix);
}

float Camera::GetFOV() const { return FOV; }
float Camera::GetAspectRatio() const { return aspectRatio; }
float Camera::GetNearPlane() const { return nearPlane; }
float Camera::GetFarPlane() const { return farPlane; }
const XMMATRIX Camera::GetWorldViewMatrix() const { return XMLoadFloat4x4(&worldView); }
const XMMATRIX Camera::GetViewMatrix() const { return XMLoadFloat4x4(&view); }
const XMMATRIX Camera::GetProjectionMatrix() const { return XMLoadFloat4x4(&projection); }

void Camera::SetFOV(float fov) { this->FOV = fov; SetProjection(this->FOV, this->aspectRatio, this->nearPlane, this->farPlane); }
void Camera::SetAspectRatio(float aspectRatio) { this->aspectRatio = aspectRatio + 0.0001f; SetProjection(this->FOV, this->aspectRatio, this->nearPlane, this->farPlane); }
void Camera::SetNearPlane(float nearPlane) { this->nearPlane = nearPlane; SetProjection(this->FOV, this->aspectRatio, this->nearPlane, this->farPlane); }
void Camera::SetFarPlane(float farPlane) { this->farPlane = farPlane; SetProjection(this->FOV, this->aspectRatio, this->nearPlane, this->farPlane); }
void Camera::SetProjection(float FOV, float aspectRatio, float nearPlane, float farPlane)
{
	this->FOV = FOV;
	this->aspectRatio = aspectRatio + 0.0001f;
	this->nearPlane = nearPlane;
	this->farPlane = farPlane;
	projectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(this->FOV), this->aspectRatio, this->nearPlane, this->farPlane);
	XMStoreFloat4x4(&projection, projectionMatrix);
}

void Camera::SetPosition(float x, float y, float z)
{
	viewMatrix = XMMatrixLookAtLH(XMVectorSet(x, y, z, 0.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	XMStoreFloat4x4(&view, viewMatrix);
	worldViewMatrix = XMMatrixInverse(nullptr, viewMatrix);
	XMStoreFloat4x4(&worldView, worldViewMatrix);
}

void Camera::IncreaseFOV(float offset)
{
	this->FOV += offset;
	if (this->FOV >= 135.0f)
		this->FOV = 135.0f;
	SetProjection(this->FOV, this->aspectRatio, this->nearPlane, this->farPlane);
}

void Camera::DecreaseFOV(float offset)
{
	this->FOV -= offset;
	if (this->FOV <= 45.0f)
		this->FOV = 45.0f;
	SetProjection(this->FOV, this->aspectRatio, this->nearPlane, this->farPlane);
}

void Camera::IncreaseNearPlane(float offset)
{
	this->nearPlane += offset;
	SetProjection(this->FOV, this->aspectRatio, this->nearPlane, this->farPlane);
}

void Camera::DecreaseNearPlane(float offset)
{
	this->nearPlane -= offset;
	if (this->nearPlane < 0.1f)
		this->nearPlane = 0.1f;
	SetProjection(this->FOV, this->aspectRatio, this->nearPlane, this->farPlane);
}

void Camera::IncreaseFarPlane(float offset)
{
	this->farPlane += offset;
	SetProjection(this->FOV, this->aspectRatio, this->nearPlane, this->farPlane);
}

void Camera::DecreaseFarPlane(float offset)
{
	this->farPlane -= offset;
	if (this->farPlane < 2.0f)
		this->farPlane = 2.0f;
	SetProjection(this->FOV, this->aspectRatio, this->nearPlane, this->farPlane);
}

void Camera::UpdateView()
{
	viewMatrix = XMMatrixInverse(nullptr, worldViewMatrix);
	XMStoreFloat4x4(&view, viewMatrix);
}

void Camera::MoveX(float offset)
{
	XMMATRIX translation = XMMatrixTranslation(offset, 0.0f, 0.0f);
	worldViewMatrix = XMMatrixMultiply(translation, worldViewMatrix);
	XMStoreFloat4x4(&worldView, worldViewMatrix);
}

void Camera::MoveY(float offset)
{
	XMMATRIX translation = XMMatrixTranslation(0.0f, offset, 0.0f);
	worldViewMatrix = XMMatrixMultiply(worldViewMatrix, translation);
	XMStoreFloat4x4(&worldView, worldViewMatrix);
}

void Camera::MoveZ(float offset)
{
	XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, offset);
	worldViewMatrix = XMMatrixMultiply(translation, worldViewMatrix);
	XMStoreFloat4x4(&worldView, worldViewMatrix);
}

void Camera::Pitch(float degrees)
{
	// local rotation
	XMMATRIX rotation = XMMatrixRotationX(degrees);
	worldViewMatrix = XMMatrixMultiply(rotation, worldViewMatrix);
	XMStoreFloat4x4(&worldView, worldViewMatrix);
}

void Camera::Yaw(float degrees)
{
	// global rotation
	XMMATRIX rotation = XMMatrixRotationY(degrees);
	XMVECTOR originalPos = worldViewMatrix.r[3];
	worldViewMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	worldViewMatrix = XMMatrixMultiply(worldViewMatrix, rotation);
	XMStoreFloat4x4(&worldView, worldViewMatrix);
	worldViewMatrix.r[3] = originalPos;
}

void Camera::Roll(float degrees)
{
}
