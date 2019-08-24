#pragma once

// Include DirectX11 and DirectXMath for interface and math operation access
#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;
#include "DDSTextureLoader.h"

#include <vector>

#include "FBXBinaryFileIO.h"


struct Vertex
{
	XMFLOAT4 position = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 tangent = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 binormal = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
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
	std::vector<ID3D11ShaderResourceView*> shaderResourceViews;
	ID3D11SamplerState* sampler = nullptr;

	//////////////////////////////
	// clean up all the resource and heap memory
	//////////////////////////////
	Mesh(char* name, float scale);
	~Mesh();

	//////////////////////////////
	// load in vertices
	//////////////////////////////
	void LoadVertices(const char* fileName);

	//////////////////////////////
	// resource initailization
	// REMARKS: MUST CALL LoadVertices BEFORE ATTEMPTING TO CREATE ANY RESOURCE
	//////////////////////////////
	HRESULT CreateVertexBuffer(ID3D11Device* device);
	HRESULT CreateIndexBuffer(ID3D11Device* device);
	HRESULT CreateVertexShader(ID3D11Device* device, const void* shaderBytecode, SIZE_T bytecodeLength);
	HRESULT CreatePixelShader(ID3D11Device* device, const void* shaderBytecode, SIZE_T bytecodeLength);
	HRESULT CreateShaderResourceView(ID3D11Device* device, const wchar_t* fileName);
	HRESULT CreateSamplerState(ID3D11Device* device);
};

inline Mesh::Mesh(char* name, float scale)
{
	this->name = name;
	this->scale = scale;
}

inline Mesh::~Mesh()
{
	// Clear heap memory
	if (vertices) { delete[] vertices; vertices = nullptr; }
	if (indices) { delete[] indices; indices = nullptr; }
	name = nullptr;
	vertexCount = 0;
	indexCount = 0;
	scale = 1.0f;

	// Release resources
	if (vertexBuffer) { vertexBuffer->Release(); vertexBuffer = nullptr; }
	if (indexBuffer) { indexBuffer->Release(); indexBuffer = nullptr; }
	if (vertexShader) { vertexShader->Release(); vertexShader = nullptr; }
	if (pixelShader) { pixelShader->Release(); pixelShader = nullptr; }
	if (sampler) { sampler->Release(); sampler = nullptr; }
	for (size_t i = 0; i < shaderResourceViews.size(); i++)
	{
		shaderResourceViews[i]->Release();
	}
}

inline void Mesh::LoadVertices(const char* fileName)
{
	FBXBinaryFileIO fileIO;
	fileIO.Read(fileName);
	this->indexCount = fileIO.indexCount;
	this->vertexCount = fileIO.vertexCount;
	this->indices = new unsigned int[this->indexCount];
	this->vertices = new Vertex[this->vertexCount];
	unsigned int i;
	for (i = 0; i < this->vertexCount; ++i)
	{
		this->vertices[i].position.x = fileIO.vertices[i].position[0] * this->scale;
		this->vertices[i].position.y = fileIO.vertices[i].position[1] * this->scale;
		this->vertices[i].position.z = fileIO.vertices[i].position[2] * this->scale;
		this->vertices[i].position.w = 1.0f;

		this->vertices[i].tangent.x = fileIO.vertices[i].tangent[0];
		this->vertices[i].tangent.y = fileIO.vertices[i].tangent[1];
		this->vertices[i].tangent.z = fileIO.vertices[i].tangent[2];
		this->vertices[i].tangent.w = fileIO.vertices[i].tangent[3];

		this->vertices[i].binormal.x = fileIO.vertices[i].binormal[0];
		this->vertices[i].binormal.y = fileIO.vertices[i].binormal[1];
		this->vertices[i].binormal.z = fileIO.vertices[i].binormal[2];
		this->vertices[i].binormal.w = fileIO.vertices[i].binormal[3];

		this->vertices[i].texture.x = fileIO.vertices[i].texture[0];
		this->vertices[i].texture.y = fileIO.vertices[i].texture[1];

		this->vertices[i].normal.x = fileIO.vertices[i].normal[0];
		this->vertices[i].normal.y = fileIO.vertices[i].normal[1];
		this->vertices[i].normal.z = fileIO.vertices[i].normal[2];
	}
	for (i = 0; i < this->indexCount; ++i)
	{
		this->indices[i] = fileIO.indices[i];
	}
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

inline HRESULT Mesh::CreateVertexShader(ID3D11Device* device, const void* shaderBytecode, SIZE_T bytecodeLength)
{
	if (device)
	{
		return device->CreateVertexShader(shaderBytecode, bytecodeLength, nullptr, &this->vertexShader);
	}
	return E_INVALIDARG;
}

inline HRESULT Mesh::CreatePixelShader(ID3D11Device* device, const void* shaderBytecode, SIZE_T bytecodeLength)
{
	if (device)
	{
		return device->CreatePixelShader(shaderBytecode, bytecodeLength, nullptr, &this->pixelShader);
	}
	return E_INVALIDARG;
}

inline HRESULT Mesh::CreateShaderResourceView(ID3D11Device* device, const wchar_t* fileName)
{
	if (device)
	{
		ID3D11ShaderResourceView* srv;
		HRESULT hr = CreateDDSTextureFromFile(device, fileName, nullptr, &srv);
		shaderResourceViews.push_back(srv);
		return hr;
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
		return device->CreateSamplerState(&sampleDesc, &this->sampler);
	}
	return E_INVALIDARG;
}
