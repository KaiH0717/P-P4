#include "Light.h"

Light::Light()
{
	position = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	positionVector = XMLoadFloat4(&position);
	normal = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	normalVector = XMLoadFloat4(&normal);
	color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	colorVector = XMLoadFloat4(&color);

	worldMatrix = XMMatrixIdentity();
	XMStoreFloat4x4(&world, worldMatrix);
}

Light::Light(const XMVECTOR& position, const XMVECTOR& normal, const XMVECTOR& color, const XMMATRIX& world)
{
	this->positionVector = position;
	this->normalVector = normal;
	this->colorVector = color;
	this->worldMatrix = world;

	XMStoreFloat4(&this->position, this->positionVector);
	XMStoreFloat4(&this->normal, this->normalVector);
	XMStoreFloat4(&this->color, this->colorVector);
	XMStoreFloat4x4(&this->world, this->worldMatrix);
}

Light::~Light()
{
}

const XMVECTOR Light::GetPositionVector() const { return XMLoadFloat4(&position); }
const XMVECTOR Light::GetNormalVector() const { return XMLoadFloat4(&normal); }
const XMVECTOR Light::GetNormalVectorNormalized() const { return XMVector4Normalize(this->GetNormalVector()); }
const XMVECTOR Light::GetColor() const { return XMLoadFloat4(&color); }
const XMMATRIX Light::GetWorldMatrix() const { return XMLoadFloat4x4(&world); }

void Light::SetPosition(const XMVECTOR& position)
{
	this->positionVector = position;
	XMStoreFloat4(&this->position, this->positionVector);
}

void Light::SetPosition(float x, float y, float z)
{
	this->position = XMFLOAT4(x, y, z, 1.0f);
	this->positionVector = XMLoadFloat4(&this->position);
}

void Light::SetNormal(const XMVECTOR& normal)
{
	this->normalVector = normal;
	XMStoreFloat4(&this->normal, this->normalVector);
}

void Light::SetNormal(float x, float y, float z)
{
	this->normal = XMFLOAT4(x, y, z, 0.0f);
	this->normalVector = XMLoadFloat4(&this->normal);
}

void Light::SetColor(const XMVECTOR& color)
{
	this->colorVector = color;
	XMStoreFloat4(&this->color, this->colorVector);
}

void Light::SetColor(float r, float g, float b, float a)
{
	this->color = XMFLOAT4(r, g, b, a);
	this->colorVector = XMLoadFloat4(&this->color);
}

void Light::SetWorldMatrix(const XMMATRIX& world)
{
	this->worldMatrix = world;
	XMStoreFloat4x4(&this->world, this->worldMatrix);
}
