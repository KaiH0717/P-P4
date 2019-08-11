#include "Graphics.h"

// shader includes
#include "VertexShader.csh"
#include "PixelShader.csh"
#include "../../My assets/StoneHenge.h"
#include "../../My assets/Artisans_Hub.h"
#include "DDSTextureLoader.h"

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
			camera.SetProjection(90.0f, 1.0f, 0.1f, 20.0f);
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
			static float rot = 0.0f; rot += 0.1f;
			// directional lighting
			//XMVECTOR newPos =
			dLight.SetPosition(0.0f, 0.0f, 0.0f);
			dLight.SetNormal(0.577f, 0.577f, -0.577f);
			dLight.SetColor(0.75f, 0.75f, 0.94f, 1.0f);
			XMStoreFloat4(&cb.lightPos[0], dLight.GetPositionVector());
			XMStoreFloat4(&cb.lightNormal[0], dLight.GetNormalVectorNormalized());
			XMStoreFloat4(&cb.lightColor[0], dLight.GetColor());
			// point light
			pLight.SetPosition(-1.0f, 0.5f, 1.0f);
			pLight.SetNormal(0.0f, 0.0f, 0.0f);
			pLight.SetColor(1.0f, 0.0f, 0.0f, 1.0f);
			XMStoreFloat4(&cb.lightPos[1], pLight.GetPositionVector());
			XMStoreFloat4(&cb.lightNormal[1], pLight.GetNormalVectorNormalized());
			XMStoreFloat4(&cb.lightColor[1], pLight.GetColor());
			// point light radius
			if (radius > 4.0f)
				shrink = true;
			else if (radius <= 1.0f)
				shrink = false;
			if (shrink)
				radius -= 0.03f;
			else
				radius += 0.03f;
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
			camera.SetAspectRatio(ar);
			//camera.SetProjection(camera.GetFOV(), ar, camera.GetNearPlane(), camera);
			XMStoreFloat4x4(&cb.projection, camera.GetProjectionMatrix());
			// map constant buffer
			D3D11_MAPPED_SUBRESOURCE gpuBuffer;
			myContext->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
			memcpy(gpuBuffer.pData, &cb, sizeof(ConstantBuffer));
			myContext->Unmap(constantBuffer, 0);

			myContext->VSSetConstantBuffers(0, 1, &constantBuffer);
			myContext->PSSetConstantBuffers(0, 1, &constantBuffer);
			myContext->PSSetShaderResources(0, 1, &shaderRV);
			myContext->PSSetSamplers(0, 1, &sampler);
			// draw
			myContext->DrawIndexed(meshes[0]->GetIndexCount(), 0, 0);

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
		{"COLOR"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	hr = myDevice->CreateInputLayout(layout, ARRAYSIZE(layout), VertexShader, sizeof(VertexShader), &vertexLayout);

	// fill vertices
	hub.SetName("ArtisansHub");
	hub.SetIndexCount(ARRAYSIZE(Artisans_Hub_indicies));
	hub.SetVertexCount(ARRAYSIZE(Artisans_Hub_data));
	hub.SetIndices((void*)Artisans_Hub_indicies);
	hub.SetVertices(new Vertex[hub.GetVertexCount()]);
	for (size_t i = 0; i < hub.GetVertexCount(); i++)
	{
		hub.GetVertices()[i].position.x = Artisans_Hub_data[i].pos[0] * 0.1f;
		hub.GetVertices()[i].position.y = Artisans_Hub_data[i].pos[1] * 0.1f;
		hub.GetVertices()[i].position.z = Artisans_Hub_data[i].pos[2] * 0.1f;
		hub.GetVertices()[i].position.w = 1.0f;

		hub.GetVertices()[i].texture.x = Artisans_Hub_data[i].uvw[0];
		hub.GetVertices()[i].texture.y = Artisans_Hub_data[i].uvw[1];

		hub.GetVertices()[i].normal.x = Artisans_Hub_data[i].nrm[0];
		hub.GetVertices()[i].normal.y = Artisans_Hub_data[i].nrm[1];
		hub.GetVertices()[i].normal.z = Artisans_Hub_data[i].nrm[2];

		hub.GetVertices()[i].color = { 1.0f, 1.0f, 1.0f, 1.0f };
	}
	meshes.push_back(&hub);
	// describe vertex data and create vertex buffer
	hr = CreateBuffer(myDevice, &vertexBuffer, D3D11_BIND_VERTEX_BUFFER, sizeof(Vertex) * hub.GetVertexCount(), hub.GetVertices());
	// describe index data and create index buffer
	hr = CreateBuffer(myDevice, &indexBuffer, D3D11_BIND_INDEX_BUFFER, sizeof(unsigned int) * hub.GetIndexCount(), hub.GetIndices());
	// describe constant variables and create constant buffer
	hr = CreateBuffer(myDevice, &constantBuffer, D3D11_BIND_CONSTANT_BUFFER, sizeof(ConstantBuffer), nullptr);
	// load texture
	hr = CreateDDSTextureFromFile(myDevice, L"../../My assets/Textures/High.dds", nullptr, &shaderRV);
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
	subRsrcData.pSysMem = pSysMem;
	switch (bindFlag)
	{
	case D3D11_BIND_VERTEX_BUFFER:
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		return device->CreateBuffer(&bufferDesc, &subRsrcData, buffer);
	case D3D11_BIND_INDEX_BUFFER:
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		return device->CreateBuffer(&bufferDesc, &subRsrcData, buffer);
	case D3D11_BIND_CONSTANT_BUFFER:
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		return device->CreateBuffer(&bufferDesc, nullptr, buffer);
	case D3D11_BIND_SHADER_RESOURCE:
		return E_NOTIMPL;
	case D3D11_BIND_STREAM_OUTPUT:
		return E_NOTIMPL;
	case D3D11_BIND_RENDER_TARGET:
		return E_NOTIMPL;
	case D3D11_BIND_DEPTH_STENCIL:
		return E_NOTIMPL;
	case D3D11_BIND_UNORDERED_ACCESS:
		return E_NOTIMPL;
	case D3D11_BIND_DECODER:
		return E_NOTIMPL;
	case D3D11_BIND_VIDEO_ENCODER:
		return E_NOTIMPL;
	default:
		return S_FALSE;
	}
}

void Graphics::KeyboardHandle()
{
	float offset = 0.025f;
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
	if (GetAsyncKeyState('J'))
	{
		camera.Rotate(0.0f, -offset, 0.0f);
	}
	// yaw right
	if (GetAsyncKeyState('L'))
	{
		camera.Rotate(0.0f, offset, 0.0f);
	}
	// pitch up
	if (GetAsyncKeyState('I'))
	{
		camera.Rotate(-offset, 0.0f, 0.0f);
	}
	// pitch down
	if (GetAsyncKeyState('K'))
	{
		camera.Rotate(offset, 0.0f, 0.0f);
	}
	// roll left
	if (GetAsyncKeyState('U'))
	{
		camera.Rotate(0.0f, 0.0f, offset);
	}
	// roll right
	if (GetAsyncKeyState('O'))
	{
		camera.Rotate(0.0f, 0.0f, -offset);
	}
	// increase fov
	if (GetAsyncKeyState('1'))
	{
		camera.IncreaseFOV(offset * 10.0f);
	}
	// decrease fov
	if (GetAsyncKeyState('2'))
	{
		camera.DecreaseFOV(offset * 10.0f);
	}
	// increase near plane
	if (GetAsyncKeyState('3'))
	{
		camera.IncreaseNearPlane(offset * 5.0f);
	}
	// decrease near plane
	if (GetAsyncKeyState('4'))
	{
		camera.DecreaseNearPlane(offset * 5.0f);
	}
	// increase far plane
	if (GetAsyncKeyState('5'))
	{
		camera.IncreaseFarPlane(offset * 5.0f);
	}
	// decrease far plane
	if (GetAsyncKeyState('6'))
	{
		camera.DecreaseFarPlane(offset * 5.0f);
	}
}
