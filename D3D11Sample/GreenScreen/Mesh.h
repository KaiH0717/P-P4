#pragma once

// Include DirectX11 and DirectXMath for interface and math operation access
#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;
#include "DDSTextureLoader.h"

#include <vector>

struct Vertex
{
	XMFLOAT4 position = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT3 normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT2 texture = XMFLOAT2(0.0f, 0.0f);
};

struct Mesh
{
	//////////////////////////////
	// attributes
	//////////////////////////////
	char* name = nullptr;
	Vertex* vertices = nullptr;
	unsigned int* indices = nullptr;
	unsigned int vertexCount = 0;
	unsigned int indexCount = 0;
	float scale = 1.0f;

	//////////////////////////////
	// buffers
	//////////////////////////////
	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* indexBuffer = nullptr;

	//////////////////////////////
	// shaders
	//////////////////////////////
	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11PixelShader* pixelShader = nullptr;

	//////////////////////////////
	// textures
	//////////////////////////////
	ID3D11ShaderResourceView* shaderRV = nullptr;
	ID3D11SamplerState* sampler = nullptr;

	~Mesh();
	// resource initailization
	HRESULT CreateVertexBuffer(ID3D11Device* device);
	HRESULT CreateIndexBuffer(ID3D11Device* device);
	HRESULT CreateVertexShader(ID3D11Device* device, const void* shaderBytecode);
	HRESULT CreatePixelShader(ID3D11Device* device, const void* shaderBytecode);
	HRESULT CreateShaderResourceView(ID3D11Device* device, const wchar_t* fileName);
	HRESULT CreateSamplerState(ID3D11Device* device);
};

inline Mesh::~Mesh()
{
	// Clear heap memory
	if (vertices) { delete[] vertices; vertices = nullptr; }
	if (indices) { delete[] indices; indices = nullptr; }
	name = nullptr;
	vertexCount = 0;
	indexCount = 0;

	// Release resources
	if (vertexBuffer) { vertexBuffer->Release(); vertexBuffer = nullptr; }
	if (indexBuffer) { indexBuffer->Release(); indexBuffer = nullptr; }
	if (vertexShader) { vertexShader->Release(); vertexShader = nullptr; }
	if (pixelShader) { pixelShader->Release(); pixelShader = nullptr; }
	if (shaderRV) { shaderRV->Release(); shaderRV = nullptr; }
	if (sampler) { sampler->Release(); sampler = nullptr; }
}

inline HRESULT Mesh::CreateVertexBuffer(ID3D11Device* device)
{
	if (device)
	{
		D3D11_BUFFER_DESC bufferDesc;
		D3D11_SUBRESOURCE_DATA subRsrcData;
		ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
		ZeroMemory(&subRsrcData, sizeof(D3D11_SUBRESOURCE_DATA));
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.ByteWidth = sizeof(Vertex) * this->vertexCount;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		subRsrcData.pSysMem = this->vertices;
		return device->CreateBuffer(&bufferDesc, &subRsrcData, &this->vertexBuffer);
	}
	return E_INVALIDARG;
}

inline HRESULT Mesh::CreateIndexBuffer(ID3D11Device* device)
{
	if (device)
	{
		D3D11_BUFFER_DESC bufferDesc;
		D3D11_SUBRESOURCE_DATA subRsrcData;
		ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
		ZeroMemory(&subRsrcData, sizeof(D3D11_SUBRESOURCE_DATA));
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.ByteWidth = sizeof(unsigned int) * this->indexCount;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		subRsrcData.pSysMem = this->indices;
		return device->CreateBuffer(&bufferDesc, &subRsrcData, &this->indexBuffer);
	}
	return E_INVALIDARG;
}

inline HRESULT Mesh::CreateVertexShader(ID3D11Device* device, const void* shaderBytecode)
{
	if (device)
	{
		return device->CreateVertexShader(shaderBytecode, sizeof(shaderBytecode), nullptr, &vertexShader);
	}
	return E_INVALIDARG;
}

inline HRESULT Mesh::CreatePixelShader(ID3D11Device* device, const void* shaderBytecode)
{
	if (device)
	{
		return device->CreatePixelShader(shaderBytecode, sizeof(shaderBytecode), nullptr, &pixelShader);
	}
	return E_INVALIDARG;
}

inline HRESULT Mesh::CreateShaderResourceView(ID3D11Device* device, const wchar_t* fileName)
{
	if (device)
	{
		return CreateDDSTextureFromFile(device, fileName, nullptr, &shaderRV);
	}
	return E_INVALIDARG;
}

inline HRESULT Mesh::CreateSamplerState(ID3D11Device* device)
{
	if (device)
	{
		D3D11_SAMPLER_DESC sampleDesc;
		ZeroMemory(&sampleDesc, sizeof(D3D11_SAMPLER_DESC));
		sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampleDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampleDesc.MinLOD = 0;
		sampleDesc.MaxLOD = D3D11_FLOAT32_MAX;
		return device->CreateSamplerState(&sampleDesc, &sampler);
	}
	return E_INVALIDARG;
}
