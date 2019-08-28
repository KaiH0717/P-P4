#include "Model.h"

void Model::SetPipeline(ID3D11DeviceContext* context, unsigned int index)
{
	UINT strides = sizeof(Vertex);
	UINT offsets = 0;
	context->IASetVertexBuffers(0, 1, &meshes[index]->vertexBuffer, &strides, &offsets);
	context->IASetIndexBuffer(meshes[index]->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	context->VSSetShader(meshes[index]->vertexShader, nullptr, 0);
	// optinal shader stages
	if (meshes[index]->geometryShader)
		context->GSSetShader(meshes[index]->geometryShader, nullptr, 0);
	else
		context->GSSetShader(nullptr, nullptr, 0);
	context->PSSetShader(meshes[index]->pixelShader, nullptr, 0);
	context->PSSetShaderResources(0, (UINT)meshes[index]->shaderResourceViews.size(), meshes[index]->shaderResourceViews.data());
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

void Model::AddMesh(Mesh* mesh, const char* fileName)
{
	if (mesh)
	{
		meshes.push_back(mesh);
		this->GetMeshes()[static_cast<unsigned int>(meshes.size() - 1)]->LoadVertices(fileName);
	}
}

void Model::DrawIndexed(ID3D11DeviceContext* context, D3D11_VIEWPORT* viewPort)
{
	if (context)
	{
		for (size_t i = 0; i < meshes.size(); i++)
		{
			SetPipeline(context, (unsigned int)i);
			context->RSSetViewports(1, viewPort);
			context->DrawIndexed(meshes[i]->indexCount, 0, 0);
		}
	}
}

void Model::DrawIndexInstanced(ID3D11DeviceContext* context, D3D11_VIEWPORT* viewPort, unsigned int numberOfInstance)
{
	if (context)
	{
		for (size_t i = 0; i < meshes.size(); i++)
		{
			SetPipeline(context, (unsigned int)i);
			context->RSSetViewports(1, viewPort);
			context->DrawIndexedInstanced(meshes[i]->indexCount, numberOfInstance, 0, 0, 0);
		}
	}
}
