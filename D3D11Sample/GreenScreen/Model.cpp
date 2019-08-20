#include "Model.h"

void Model::SetPipeline(ID3D11DeviceContext* context, unsigned int index)
{
	UINT strides = sizeof(Vertex);
	UINT offsets = 0;
	context->IASetVertexBuffers(0, 1, &meshes[index]->vertexBuffer, &strides, &offsets);
	context->IASetIndexBuffer(meshes[index]->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	context->VSSetShader(meshes[index]->vertexShader, nullptr, 0);
	context->PSSetShader(meshes[index]->pixelShader, nullptr, 0);
	context->PSSetShaderResources(0, 1, &meshes[index]->shaderRV);
	context->PSSetSamplers(0, 1, &meshes[index]->sampler);
}

Model::Model()
{
	name = nullptr;
	XMStoreFloat4x4(&world, XMMatrixIdentity());
}

Model::~Model()
{
	for (size_t i = 0; i < meshes.size(); i++)
	{
		delete meshes[i];
	}
	meshes.clear();
}

char* Model::GetName() { return name; }
std::vector<Mesh*>& Model::GetMeshes() { return meshes; }
XMMATRIX Model::GetWorldMatrix() { return XMLoadFloat4x4(&world); }

void Model::SetName(char* name) { this->name = name; }
void Model::SetWorldMatrix(const XMMATRIX& world) { XMStoreFloat4x4(&this->world, world); }

void Model::AddMesh(Mesh* mesh)
{
	if (mesh)
	{
		meshes.push_back(mesh);
	}
}

void Model::Draw(ID3D11DeviceContext* context)
{
	if (context)
	{
		for (size_t i = 0; i < meshes.size(); i++)
		{
			SetPipeline(context, i);
			context->DrawIndexed(meshes[i]->indexCount, 0, 0);
		}
	}
}

void Model::Draw(ID3D11DeviceContext* context, unsigned int index)
{
	if (context && (index >= 0 && index < meshes.size()))
	{
		SetPipeline(context, index);
		context->DrawIndexed(meshes[index]->vertexCount, 0, 0);
	}
}
