#include "Graphics.h"

// shader includes
#include "VertexShader.csh"
#include "PixelShader.csh"
#include "../../My assets/StoneHenge.h"
#include "../../My assets/lionheart.h"
#include "DDSTextureLoader.h"
#include <iostream>

Graphics::Graphics(GW::SYSTEM::GWindow* attatchPoint)
{
	if (attatchPoint) // valid window?
	{
		// Create surface, will auto attatch to GWindow
		if (G_SUCCESS(GW::GRAPHICS::CreateGDirectX11Surface(attatchPoint, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT, &mySurface)))
		{
			// Grab handles to all DX11 base interfaces
			mySurface->GetDevice((void**)& myDevice);
			mySurface->GetSwapchain((void**)& mySwapChain);
			mySurface->GetContext((void**)& myContext);

			// Initalize device
			HRESULT hr = InitializeDevice();
			camera.SetPosition(0.0f, 2.0f, -5.0f);
		}
	}
}

Graphics::~Graphics()
{
	// Release DX Objects aquired from the surface
	myDevice->Release();
	mySwapChain->Release();
	myContext->Release();

	// TODO: "Release()" more stuff here!
	CleanDevice();

	if (mySurface) // Free Gateware Interface
	{
		mySurface->DecrementCount(); // reduce internal count (will auto delete on Zero)
		mySurface = nullptr; // the safest way to fly
	}
}

void Graphics::Render()
{
	if (mySurface) // valid?
	{
		// this could be changed during resolution edits, get it every frame
		ID3D11RenderTargetView* myRenderTargetView = nullptr;
		ID3D11DepthStencilView* myDepthStencilView = nullptr;
		if (G_SUCCESS(mySurface->GetRenderTarget((void**)& myRenderTargetView)))
		{
			// Grab the Z Buffer if one was requested
			if (G_SUCCESS(mySurface->GetDepthStencilView((void**)& myDepthStencilView)))
			{
				myContext->ClearDepthStencilView(myDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0); // clear it to Z exponential Far.
				myDepthStencilView->Release();
			}

			// Set active target for drawing, all array based D3D11 functions should use a syntax similar to below
			ID3D11RenderTargetView* const targets[] = { myRenderTargetView };
			myContext->OMSetRenderTargets(1, targets, myDepthStencilView);
			// Clear screen
			const float bg_Color[] = { 0, 0, 0, 1 };
			myContext->ClearRenderTargetView(myRenderTargetView, bg_Color);
			
			// keyboard inputs
			KeyboardHandle();

			// setup pipeline
			UINT strides = sizeof(Vertex);
			UINT offsets = 0;
			myContext->IASetInputLayout(vertexLayout);
			myContext->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offsets);
			myContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
			myContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			myContext->VSSetShader(vertexShader, nullptr, 0);
			myContext->PSSetShader(pixelShader, nullptr, 0);
			myContext->OMSetRenderTargets(1, &myRenderTargetView, nullptr);

			// lighting
			// directional lighting
			XMVECTOR pos = { 0.0f, 0.0f, 0.0f, 1.0f };
			XMVECTOR nor = { 0.577f, 0.577f, -0.577f, 0.0f };
			XMVECTOR col = { 0.75f, 0.75f, 0.94f, 1.0f };
			XMStoreFloat4(&cb.lightPos[0], pos);
			XMStoreFloat4(&cb.lightNormal[0], XMVector4Normalize(nor));
			XMStoreFloat4(&cb.lightColor[0], col);
			// point light
			pos = { -1.0f, 0.5f, 1.0f, 1.0f };
			nor = { 0.0f, 0.0f, 0.0f, 0.0f };
			col = { 1.0f, 0.0f, 0.0f, 1.0f };
			XMStoreFloat4(&cb.lightPos[1], pos);
			XMStoreFloat4(&cb.lightNormal[1], XMVector4Normalize(nor));
			XMStoreFloat4(&cb.lightColor[1], col);
			// point light radius
			if (radius > 5.0f)
				shrink = true;
			else if (radius <= 1.0f)
				shrink = false;
			if (shrink)
				radius -= 0.01f;
			else
				radius += 0.01f;
			XMVECTOR lightRad = { radius, 0.0f, 0.0f, 0.0f };
			XMStoreFloat4(&cb.lightRadius, lightRad);

			// world
			XMMATRIX temp = XMMatrixIdentity();
			temp = XMMatrixTranslation(0.0f, 0.0f, 0.5f);
			XMStoreFloat4x4(&cb.world, temp);
			// view
			XMStoreFloat4x4(&cb.view, camera.GetViewMatrix());
			// projection
			float ar = 0.0f;
			mySurface->GetAspectRatio(ar);
			camera.SetProjection(90.0f, ar, 0.1f, 100.0f);
			XMStoreFloat4x4(&cb.projection, camera.GetProjectionMatrix());
			// map constant buffer
			D3D11_MAPPED_SUBRESOURCE gpuBuffer;
			myContext->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
			memcpy(gpuBuffer.pData, &cb, sizeof(ConstantBuffer));
			myContext->Unmap(constantBuffer, 0);
			ID3D11Buffer* constants[] = { constantBuffer };

			myContext->VSSetConstantBuffers(0, 1, constants);
			myContext->PSSetConstantBuffers(0, 1, constants);
			myContext->PSSetShaderResources(0, 1, &shaderRV);
			myContext->PSSetSamplers(0, 1, &sampler);
			// draw
			myContext->DrawIndexed(meshes[0].GetIndexCount(), 0, 0);

			// Present Backbuffer using Swapchain object
			// Framerate is currently unlocked, we suggest "MSI Afterburner" to track your current FPS and memory usage.
			mySwapChain->Present(1, 0); // set first argument to 1 to enable vertical refresh sync with display
			// Free any temp DX handles aquired this frame
			myRenderTargetView->Release();
		}
	}
}

HRESULT Graphics::InitializeDevice()
{
	HRESULT hr = S_OK;

	// write and compile & load our shaders
	hr = myDevice->CreateVertexShader(VertexShader, sizeof(VertexShader), nullptr, &vertexShader);
	hr = myDevice->CreatePixelShader(PixelShader, sizeof(PixelShader), nullptr, &pixelShader);

	// define input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT	  , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT	  , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	hr = myDevice->CreateInputLayout(layout, ARRAYSIZE(layout), VertexShader, sizeof(VertexShader), &vertexLayout);

	// fill vertices
	Mesh mesh("StoneHenge", (void*)StoneHenge_indicies, ARRAYSIZE(StoneHenge_data), ARRAYSIZE(StoneHenge_indicies));
	meshes.push_back(mesh);
	for (size_t i = 0; i < mesh.GetVertexCount(); i++)
	{
		mesh.GetVertices()[i].position.x = StoneHenge_data[i].pos[0] * 0.1f;
		mesh.GetVertices()[i].position.y = StoneHenge_data[i].pos[1] * 0.1f;
		mesh.GetVertices()[i].position.z = StoneHenge_data[i].pos[2] * 0.1f;
		mesh.GetVertices()[i].position.w = 1.0f;

		mesh.GetVertices()[i].texture.x = StoneHenge_data[i].uvw[0];
		mesh.GetVertices()[i].texture.y = StoneHenge_data[i].uvw[1];

		mesh.GetVertices()[i].normal.x = StoneHenge_data[i].nrm[0];
		mesh.GetVertices()[i].normal.y = StoneHenge_data[i].nrm[1];
		mesh.GetVertices()[i].normal.z = StoneHenge_data[i].nrm[2];

		mesh.GetVertices()[i].color = { 1.0f, 1.0f, 1.0f, 1.0f };
	}
	// describe vertex data and create vertex buffer
	hr = CreateBuffer(myDevice, &vertexBuffer, D3D11_BIND_VERTEX_BUFFER, sizeof(Vertex) * mesh.GetVertexCount(), mesh.GetVertices());
	// describe index data and create index buffer
	hr = CreateBuffer(myDevice, &indexBuffer, D3D11_BIND_INDEX_BUFFER, sizeof(unsigned int) * mesh.GetIndexCount(), mesh.GetIndices());
	// describe constant variables and create constant buffer
	hr = CreateBuffer(myDevice, &constantBuffer, D3D11_BIND_CONSTANT_BUFFER, sizeof(ConstantBuffer), nullptr);
	// load texture
	hr = CreateDDSTextureFromFile(myDevice, L"../../My assets/StoneHenge.dds", nullptr, &shaderRV);
	// create sample
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(D3D11_SAMPLER_DESC));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = myDevice->CreateSamplerState(&sampDesc, &sampler);
	return hr;
}

void Graphics::CleanDevice()
{
	constantBuffer->Release();

	vertexBuffer->Release();
	indexBuffer->Release();
	vertexLayout->Release();
	vertexShader->Release();
	pixelShader->Release();

	shaderRV->Release();
	sampler->Release();
}

HRESULT Graphics::CreateBuffer(ID3D11Device* device, ID3D11Buffer** buffer, UINT bindFlag, UINT byteWidth, const void* pSysMem)
{
	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA subRsrcData;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&subRsrcData, sizeof(D3D11_SUBRESOURCE_DATA));
	bufferDesc.BindFlags = bindFlag;
	bufferDesc.ByteWidth = byteWidth;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	switch (bindFlag)
	{
	case D3D11_BIND_VERTEX_BUFFER:
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		subRsrcData.pSysMem = pSysMem;
		device->CreateBuffer(&bufferDesc, &subRsrcData, buffer);
		return S_OK;
	case D3D11_BIND_INDEX_BUFFER:
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		subRsrcData.pSysMem = pSysMem;
		device->CreateBuffer(&bufferDesc, &subRsrcData, buffer);
		return S_OK;
	case D3D11_BIND_CONSTANT_BUFFER:
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		device->CreateBuffer(&bufferDesc, nullptr, buffer);
		return S_OK;
	default:
		return S_FALSE;
	}
}

void Graphics::KeyboardHandle()
{
	float offset = 0.01f;
	// move forward
	if (GetAsyncKeyState('W'))
	{
		camera.Move(0.0f, 0.0f, offset);
	}
	// move left
	if (GetAsyncKeyState('A'))
	{
		camera.Move(-offset, 0.0f, 0.0f);
	}
	// move backwards
	if (GetAsyncKeyState('S'))
	{
		camera.Move(0.0f, 0.0f, -offset);
	}
	// move right
	if (GetAsyncKeyState('D'))
	{
		camera.Move(offset, 0.0f, 0.0f);
	}
	// move up
	if (GetAsyncKeyState('Q'))
	{
		camera.Move(0.0f, offset, 0.0f);
	}
	// move down
	if (GetAsyncKeyState('E'))
	{
		camera.Move(0.0f, -offset, 0.0f);
	}
	// yaw left
	if (GetAsyncKeyState(VK_LEFT))
	{
		camera.Rotate(0.0f, -offset, 0.0f);
	}
	// yaw right
	if (GetAsyncKeyState(VK_RIGHT))
	{
		camera.Rotate(0.0f, offset, 0.0f);
	}
	// pitch up
	if (GetAsyncKeyState(VK_UP))
	{
		camera.Rotate(-offset, 0.0f, 0.0f);
	}
	// pitch down
	if (GetAsyncKeyState(VK_DOWN))
	{
		camera.Rotate(offset, 0.0f, 0.0f);
	}
	if (GetAsyncKeyState('R'))
	{
		camera.Reset();
	}
}
