#include "Camera.h"

Camera::Camera()
{
	FOV = 90.0f;
	aspectRatio = 1.0f;
	nearPlane = 0.1f;
	farPlane = 10.0f;

	XMStoreFloat4x4(&view, XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
	XMStoreFloat4x4(&worldView, XMMatrixInverse(nullptr, XMLoadFloat4x4(&view)));
	XMStoreFloat4x4(&projection, XMMatrixIdentity());
}

float Camera::GetFOV() const { return FOV; }
float Camera::GetAspectRatio() const { return aspectRatio; }
float Camera::GetNearPlane() const { return nearPlane; }
float Camera::GetFarPlane() const { return farPlane; }
const XMMATRIX Camera::GetViewMatrix() const { return XMLoadFloat4x4(&view); }
const XMMATRIX Camera::GetProjectionMatrix() const { return XMLoadFloat4x4(&projection); }
const XMFLOAT4 Camera::GetWorldPosition() const { return XMFLOAT4(worldView._41, worldView._42, worldView._43, 1.0f); }
const XMFLOAT4 Camera::GetLocalPosition() const { return XMFLOAT4(view._41, view._42, view._43, 1.0f); }

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
	XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(this->FOV), this->aspectRatio, this->nearPlane, this->farPlane);
	XMStoreFloat4x4(&projection, projectionMatrix);
}

void Camera::SetPosition(float x, float y, float z)
{
	XMMATRIX viewMatrix = XMMatrixLookAtLH(XMVectorSet(x, y, z, 0.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	XMStoreFloat4x4(&view, viewMatrix);
	XMMATRIX worldViewMatrix = XMMatrixInverse(nullptr, viewMatrix);
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
	XMMATRIX viewMatrix = XMMatrixInverse(nullptr, XMLoadFloat4x4(&worldView));
	XMStoreFloat4x4(&view, viewMatrix);
}

void Camera::MoveX(float offset)
{
	XMMATRIX translation = XMMatrixTranslation(offset, 0.0f, 0.0f);
	XMMATRIX worldViewMatrix = XMMatrixMultiply(translation, XMLoadFloat4x4(&worldView));
	XMStoreFloat4x4(&worldView, worldViewMatrix);
}

void Camera::MoveY(float offset)
{
	XMMATRIX translation = XMMatrixTranslation(0.0f, offset, 0.0f);
	XMMATRIX worldViewMatrix = XMMatrixMultiply(XMLoadFloat4x4(&worldView), translation);
	XMStoreFloat4x4(&worldView, worldViewMatrix);
}

void Camera::MoveZ(float offset)
{
	XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, offset);
	XMMATRIX worldViewMatrix = XMMatrixMultiply(translation, XMLoadFloat4x4(&worldView));
	XMStoreFloat4x4(&worldView, worldViewMatrix);
}

void Camera::Pitch(float degrees)
{
	// local rotation
	XMMATRIX rotation = XMMatrixRotationX(degrees);
	XMMATRIX worldViewMatrix = XMMatrixMultiply(rotation, XMLoadFloat4x4(&worldView));
	XMStoreFloat4x4(&worldView, worldViewMatrix);
}

void Camera::Yaw(float degrees)
{
	// global rotation
	XMMATRIX rotation = XMMatrixRotationY(degrees);
	XMMATRIX worldViewMatrix = XMLoadFloat4x4(&worldView);
	XMVECTOR originalPos = worldViewMatrix.r[3];
	worldViewMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	worldViewMatrix = XMMatrixMultiply(worldViewMatrix, rotation);
	worldViewMatrix.r[3] = originalPos;
	XMStoreFloat4x4(&worldView, worldViewMatrix);
}

void Camera::Roll(float degrees)
{
	XMMATRIX rotation = XMMatrixRotationZ(degrees);
	XMMATRIX worldViewMatrix = XMMatrixMultiply(rotation, XMLoadFloat4x4(&worldView));
	XMStoreFloat4x4(&worldView, worldViewMatrix);
}
