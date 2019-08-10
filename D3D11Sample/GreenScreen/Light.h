#pragma once

#include <DirectXMath.h>
using namespace DirectX;

class Light
{
private:
	// storage type
	XMFLOAT4 position;
	XMFLOAT4 normal;
	XMFLOAT4 color;
	XMFLOAT4X4 world;
	// speed type
	XMVECTOR positionVector;
	XMVECTOR normalVector;
	XMVECTOR colorVector;
	XMMATRIX worldMatrix;
public:
	Light();
	Light(const XMVECTOR& position, const XMVECTOR& normal, const XMVECTOR& color, const XMMATRIX& world);
	~Light();

	const XMVECTOR GetPositionVector() const;
	const XMVECTOR GetNormalVector() const;
	const XMVECTOR GetNormalVectorNormalized() const;
	const XMVECTOR GetColor() const;
	const XMMATRIX GetWorldMatrix() const;

	void SetPosition(const XMVECTOR& position);
	void SetPosition(float x, float y, float z);
	void SetNormal(const XMVECTOR& normal);
	void SetNormal(float x, float y, float z);
	void SetColor(const XMVECTOR& color);
	void SetColor(float r, float g, float b, float a);
	void SetWorldMatrix(const XMMATRIX& world);
};
