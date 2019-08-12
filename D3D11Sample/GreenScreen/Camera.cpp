#include "Camera.h"

Camera::Camera()
{
	FOV = 90.0f;
	aspectRatio = 1.0f;
	nearPlane = 0.1f;
	farPlane = 10.0f;

	position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	positionVector = XMLoadFloat3(&position);
	rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	rotationVector = XMLoadFloat3(&rotation);

	viewMatrix = XMMatrixIdentity();
	XMStoreFloat4x4(&view, viewMatrix);
	projectionMatrix = XMMatrixIdentity();
	XMStoreFloat4x4(&projection, projectionMatrix);
	UpdateView();
}

Camera::~Camera()
{
}

void Camera::Reset()
{
	this->SetPosition(0.0f, 0.0f, -1.0f);
	this->Rotate(0.0f, 0.0f, 0.0f);
}

float Camera::GetFOV() const { return FOV; }
float Camera::GetAspectRatio() const { return aspectRatio; }
float Camera::GetNearPlane() const { return nearPlane; }
float Camera::GetFarPlane() const { return farPlane; }
const XMMATRIX Camera::GetViewMatrix() const { return XMLoadFloat4x4(&view); }
const XMMATRIX Camera::GetProjectionMatrix() const { return XMLoadFloat4x4(&projection); }
const XMVECTOR Camera::GetPositionVector() const { return XMLoadFloat3(&position); }
const XMVECTOR Camera::GetRotationVector() const { return XMLoadFloat3(&rotation); }

void Camera::SetFOV(float fov)
{
	this->FOV = fov;
	SetProjection(this->FOV, this->aspectRatio, this->nearPlane, this->farPlane);
}

void Camera::SetAspectRatio(float aspectRatio)
{
	this->aspectRatio = aspectRatio;
	SetProjection(this->FOV, this->aspectRatio, this->nearPlane, this->farPlane);
}

void Camera::SetNearPlane(float nearPlane)
{
	this->nearPlane = nearPlane;
	SetProjection(this->FOV, this->aspectRatio, this->nearPlane, this->farPlane);
}

void Camera::SetFarPlane(float farPlane)
{
	this->farPlane = farPlane;
	SetProjection(this->FOV, this->aspectRatio, this->nearPlane, this->farPlane);
}

void Camera::SetProjection(float FOV, float aspectRatio, float nearPlane, float farPlane)
{
	this->FOV = FOV;
	this->aspectRatio = aspectRatio;
	this->nearPlane = nearPlane;
	this->farPlane = farPlane;
	projectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(this->FOV), this->aspectRatio, this->nearPlane, this->farPlane);
	XMStoreFloat4x4(&projection, projectionMatrix);
}

void Camera::SetPosition(const XMVECTOR& position)
{
	this->positionVector = position;
	XMStoreFloat3(&this->position, this->positionVector);
	UpdateView();
}

void Camera::SetPosition(float x, float y, float z)
{
	this->position = XMFLOAT3(x, y, z);
	this->positionVector = XMLoadFloat3(&this->position);
	UpdateView();
}

void Camera::Move(const XMVECTOR& position)
{
	this->positionVector += position;
	XMStoreFloat3(&this->position, this->positionVector);
	UpdateView();
}

void Camera::Move(float x, float y, float z)
{
	this->position.x += x;
	this->position.y += y;
	this->position.z += z;
	this->positionVector = XMLoadFloat3(&this->position);
	UpdateView();
}

void Camera::Rotate(const XMVECTOR& rotation)
{
	this->rotationVector += rotation;
	XMStoreFloat3(&this->rotation, this->rotationVector);
	UpdateView();
}

void Camera::Rotate(float x, float y, float z)
{
	this->rotation.x += x;
	this->rotation.y += y;
	this->rotation.z += z;
	this->rotationVector = XMLoadFloat3(&this->rotation);
	UpdateView();
}

void Camera::IncreaseFOV(float offset)
{
	this->FOV += offset;
	SetProjection(this->FOV, this->aspectRatio, this->nearPlane, this->farPlane);
}

void Camera::DecreaseFOV(float offset)
{
	this->FOV -= offset;
	if (this->FOV < 45.0f)
		this->FOV = 45.0f;
	SetProjection(this->FOV, this->aspectRatio, this->nearPlane, this->farPlane);
}

void Camera::IncreaseNearPlane(float offset)
{
	this->nearPlane += offset;
	if (this->FOV > 135.0f)
		this->FOV = 135.0f;
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
	// calculate camera rotation
	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(this->rotation.x, this->rotation.y, this->rotation.z);
	// calculate unit vector of cam target based off camera forward value transformed by cam rotation matrix
	XMVECTOR target = XMVector3TransformCoord(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rotation);
	// adjust cam target to be offset by the camera's current position
	target += this->positionVector;
	// calculate up direction based on current rotation
	XMVECTOR up = XMVector3TransformCoord(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), rotation);
	// rebuild view matrix
	this->viewMatrix = XMMatrixLookAtLH(this->positionVector, target, up);
	XMStoreFloat4x4(&this->view, this->viewMatrix);
}
