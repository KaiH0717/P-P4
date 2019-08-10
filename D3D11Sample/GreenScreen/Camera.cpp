#include "Camera.h"

Camera::Camera()
{
	this->position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	this->positionVector = XMLoadFloat3(&position);
	this->rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	this->rotationVector = XMLoadFloat3(&rotation);
	this->UpdateViewMatrix();
}

void Camera::SetProjectionValues(float fovDegrees, float aspectRatio, float nearZ, float farZ)
{
	float fovRadians = (fovDegrees / 360.0f) * XM_2PI;
	this->projectionMatrix = XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, nearZ, farZ);
}

const XMMATRIX& Camera::GetViewMatrix() const
{
	return viewMatrix;
}

const XMMATRIX& Camera::GetProjectionMatrix() const
{
	return projectionMatrix;
}

const XMVECTOR& Camera::GetPositionVector() const
{
	return positionVector;
}

const XMFLOAT3& Camera::GetPositionFloat3() const
{
	return position;
}

const XMVECTOR& Camera::GetRotationVector() const
{
	return rotationVector;
}

const XMFLOAT3& Camera::GetRotationFloat3() const
{
	return rotation;
}

void Camera::SetPosition(const XMVECTOR& pos)
{
	XMStoreFloat3(&this->position, pos);
	this->positionVector = pos;
	this->UpdateViewMatrix();
}

void Camera::SetPosition(float x, float y, float z)
{
	this->position = XMFLOAT3(x, y, z);
	this->positionVector = XMLoadFloat3(&this->position);
	this->UpdateViewMatrix();
}

void Camera::Move(const XMVECTOR& pos)
{
	this->positionVector += pos;
	XMStoreFloat3(&this->position, this->positionVector);
	this->UpdateViewMatrix();
}

void Camera::Move(float x, float y, float z)
{
	this->position.x += x;
	this->position.y += y;
	this->position.z += z;
	this->positionVector = XMLoadFloat3(&this->position);
	this->UpdateViewMatrix();
}

void Camera::Rotate(const XMVECTOR& rot)
{
	this->rotationVector += rot;
	XMStoreFloat3(&this->rotation, this->rotationVector);
	this->UpdateViewMatrix();
}

void Camera::Rotate(float x, float y, float z)
{
	this->rotation.x += x;
	this->rotation.y += y;
	this->rotation.z += z;
	this->rotationVector = XMLoadFloat3(&this->rotation);
	this->UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
	//Calculate camera rotation matrix
	XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(this->rotation.x, this->rotation.y, this->rotation.z);
	//Calculate unit vector of cam target based off camera forward value transformed by cam rotation matrix
	XMVECTOR camTarget = XMVector3TransformCoord(this->DEFAULT_FORWARD_VECTOR, camRotationMatrix);
	//Adjust cam target to be offset by the camera's current position
	camTarget += this->positionVector;
	//Calculate up direction based on current rotation
	XMVECTOR upDir = XMVector3TransformCoord(this->DEFAULT_UP_VECTOR, camRotationMatrix);
	//Rebuild view matrix
	this->viewMatrix = XMMatrixLookAtLH(this->positionVector, camTarget, upDir);
}
