#include "Camera.h"

Camera::Camera()
{
	position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	positionVector = XMLoadFloat3(&position);
	rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	rotationVector = XMLoadFloat3(&rotation);
	view = XMFLOAT4X4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	viewMatrix = XMLoadFloat4x4(&view);
	projection = XMFLOAT4X4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	projectionMatrix = XMLoadFloat4x4(&projection);
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

const XMMATRIX Camera::GetViewMatrix() const { return XMLoadFloat4x4(&this->view); }
const XMMATRIX Camera::GetProjectionMatrix() const { return XMLoadFloat4x4(&this->projection); }
const XMVECTOR Camera::GetPositionVector() const { return XMLoadFloat3(&this->position); }
const XMVECTOR Camera::GetRotationVector() const { return XMLoadFloat3(&this->rotation); }

void Camera::SetProjection(float fov, float aspectRatio, float nearPlane, float farPlane)
{
	projectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(fov), aspectRatio, nearPlane, farPlane);
	XMStoreFloat4x4(&projection, projectionMatrix);
}

void Camera::SetPosition(const XMVECTOR& pos)
{
	this->positionVector = pos;
	XMStoreFloat3(&this->position, this->positionVector);
	UpdateView();
}

void Camera::SetPosition(float x, float y, float z)
{
	this->position = XMFLOAT3(x, y, z);
	this->positionVector = XMLoadFloat3(&this->position);
	UpdateView();
}

void Camera::Move(const XMVECTOR& pos)
{
	this->positionVector += pos;
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

void Camera::Rotate(const XMVECTOR& rot)
{
	this->rotationVector += rot;
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

void Camera::UpdateView()
{
	//Calculate camera rotation matrix
	XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(this->rotation.x, this->rotation.y, this->rotation.z);
	//Calculate unit vector of cam target based off camera forward value transformed by cam rotation matrix
	XMVECTOR camTarget = XMVector3TransformCoord(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), camRotationMatrix);
	//Adjust cam target to be offset by the camera's current position
	camTarget += this->positionVector;
	//Calculate up direction based on current rotation
	XMVECTOR upDir = XMVector3TransformCoord(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), camRotationMatrix);
	//Rebuild view matrix
	this->viewMatrix = XMMatrixLookAtLH(this->positionVector, camTarget, upDir);
	XMStoreFloat4x4(&this->view, this->viewMatrix);
}
