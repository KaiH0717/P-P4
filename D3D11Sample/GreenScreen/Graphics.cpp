#include "Graphics.h"

// shader includes
#include "VertexShader.csh"
#include "PixelShader.csh"

#define RAND_COLOR XMFLOAT4(rand()/float(RAND_MAX),rand()/float(RAND_MAX),rand()/float(RAND_MAX),1.0f)

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
			camera.SetPosition(0.0f, 0.0f, -20.0f);
			camera.SetProjection(90.0f, 1.0f, 0.1f, 500.0f);
			pLight.SetPosition(0.0f, 5.0f, 0.0f);
			pLight.SetNormal(0.0f, 0.0f, 0.0f);
			dLight.SetPosition(0.0f, 0.0f, 0.0f);
			dLight.SetNormal(0.577f, 0.577f, -0.577f);
			time.Restart();
			elapsedTime = 0;
			wave = false;
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
			const float bg_Color[] = { 0.0f, 0.0f, 0.4f, 1.0f };
			myContext->ClearRenderTargetView(myRenderTargetView, bg_Color);

			// keyboard inputs
			time.Signal();
			elapsedTime += time.Delta();
			KeyboardHandle((float)time.SmoothDelta());

			// setup pipeline
			UINT strides = sizeof(Vertex);
			UINT offsets = 0;
			myContext->IASetInputLayout(vertexLayout);
			myContext->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offsets);
			myContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
			myContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			myContext->VSSetShader(vertexShader, nullptr, 0);
			myContext->PSSetShader(pixelShader, nullptr, 0);

			// lighting
			static float rot = 0.0f; rot += 0.1f;
			// directional lighting
			dLight.SetWorldMatrix(XMMatrixRotationY(0.01f));
			dLight.UpdatePositionVector();
			dLight.UpdateNormalVector();
			dLight.SetColor(0.75f, 0.75f, 0.94f, 1.0f);
			XMStoreFloat4(&cb.lightPos[0], dLight.GetPositionVector());
			XMStoreFloat4(&cb.lightNormal[0], dLight.GetNormalVectorNormalized());
			XMStoreFloat4(&cb.lightColor[0], dLight.GetColor());
			// point light
			pLight.SetWorldMatrix(XMMatrixRotationY(0.01f));
			pLight.UpdatePositionVector();
			pLight.UpdateNormalVector();
			pLight.SetColor(1.0f, 0.0f, 0.0f, 1.0f);
			XMStoreFloat4(&cb.lightPos[1], pLight.GetPositionVector());
			XMStoreFloat4(&cb.lightNormal[1], pLight.GetNormalVectorNormalized());
			XMStoreFloat4(&cb.lightColor[1], pLight.GetColor());
			// point light radius
			if (radius > 10.0f)
				shrink = true;
			else if (radius <= 1.0f)
				shrink = false;
			if (shrink)
				radius -= 0.03f;
			else
				radius += 0.03f;
			XMVECTOR lightRad = { radius, rot, (float)elapsedTime, (float)wave };
			XMStoreFloat4(&cb.lightRadius, lightRad);

			// world
			XMMATRIX temp = XMMatrixIdentity();
			//temp = XMMatrixRotationY(90.0f * 0.1f);
			//temp = XMMatrixMultiply(XMMatrixRotationZ(60.0f * 0.1f), temp);
			//temp = XMMatrixMultiply(XMMatrixRotationX(90.0f * 0.1f), temp);
			XMStoreFloat4x4(&cb.world, temp);
			// view
			XMStoreFloat4x4(&cb.view, camera.GetViewMatrix());
			// projection
			float ar = 0.0f;
			mySurface->GetAspectRatio(ar);
			camera.SetAspectRatio(ar);
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
			myContext->DrawIndexed(hub.indexCount, 0, 0);
			//myContext->DrawIndexed(meshes[0]->indexCount, 0, 0);

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
		{"COLOR"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT	  , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT	  , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	hr = myDevice->CreateInputLayout(layout, ARRAYSIZE(layout), VertexShader, sizeof(VertexShader), &vertexLayout);

	// load data from binary file
	fileIO.Read("../../My assets/Corvette_Data");
	float scale = 0.005f;
	// fill vertices
	hub.name = "ArtisansHub";
	hub.indexCount = fileIO.indexCount;
	hub.vertexCount = fileIO.vertexCount;
	hub.indices = new unsigned int[hub.indexCount];
	hub.vertices = new Vertex[hub.vertexCount];
	for (size_t i = 0; i < hub.vertexCount; i++)
	{
		hub.vertices[i].position.x = fileIO.vertices[i].position[0] * scale;
		hub.vertices[i].position.y = fileIO.vertices[i].position[1] * scale;
		hub.vertices[i].position.z = fileIO.vertices[i].position[2] * scale;
		hub.vertices[i].position.w = 1.0f;

		hub.vertices[i].texture.x = fileIO.vertices[i].texture[0];
		hub.vertices[i].texture.y = fileIO.vertices[i].texture[1];

		hub.vertices[i].normal.x = fileIO.vertices[i].normal[0];
		hub.vertices[i].normal.y = fileIO.vertices[i].normal[1];
		hub.vertices[i].normal.z = fileIO.vertices[i].normal[2];

		hub.vertices[i].color = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	}
	for (size_t i = 0; i < hub.indexCount; i++)
	{
		hub.indices[i] = fileIO.indices[i];
	}
	meshes.push_back(&hub);
	// describe vertex data and create vertex buffer
	hr = CreateBuffer(myDevice, &vertexBuffer, D3D11_BIND_VERTEX_BUFFER, sizeof(Vertex) * hub.vertexCount, hub.vertices);
	// describe index data and create index buffer
	hr = CreateBuffer(myDevice, &indexBuffer, D3D11_BIND_INDEX_BUFFER, sizeof(unsigned int) * hub.indexCount, hub.indices);
	// describe constant variables and create constant buffer
	hr = CreateBuffer(myDevice, &constantBuffer, D3D11_BIND_CONSTANT_BUFFER, sizeof(ConstantBuffer), nullptr);
	// load texture
	hr = CreateDDSTextureFromFile(myDevice, L"../../My assets/Textures/SF_Corvette-F3_diffuse.dds", nullptr, &shaderRV);
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

void Graphics::KeyboardHandle(float delta)
{
	float offset = 3.5f;
	// move forward
	if (GetAsyncKeyState('W'))
	{
		camera.MoveZ(offset * delta);
	}
	// move left
	if (GetAsyncKeyState('A'))
	{
		camera.MoveX(-offset * delta);
	}
	// move backwards
	if (GetAsyncKeyState('S'))
	{
		camera.MoveZ(-offset * delta);
	}
	// move right
	if (GetAsyncKeyState('D'))
	{
		camera.MoveX(offset * delta);
	}
	// move up
	if (GetAsyncKeyState('Q'))
	{
		camera.MoveY(offset * delta);
	}
	// move down
	if (GetAsyncKeyState('E'))
	{
		camera.MoveY(-offset * delta);
	}
	// yaw left
	if (GetAsyncKeyState('J'))
	{
		camera.Yaw(-offset * delta);
	}
	// yaw right
	if (GetAsyncKeyState('L'))
	{
		camera.Yaw(offset * delta);
	}
	// pitch up
	if (GetAsyncKeyState('I'))
	{
		camera.Pitch(-offset * delta);
	}
	// pitch down
	if (GetAsyncKeyState('K'))
	{
		camera.Pitch(offset * delta);
	}
	// increase fov
	if (GetAsyncKeyState('1'))
	{
		camera.IncreaseFOV((offset + 25.0f) * delta);
	}
	// decrease fov
	if (GetAsyncKeyState('2'))
	{
		camera.DecreaseFOV((offset + 25.0f) * delta);
	}
	// increase near plane
	if (GetAsyncKeyState('3'))
	{
		camera.IncreaseNearPlane((offset + 10.0f) * delta);
	}
	// decrease near plane
	if (GetAsyncKeyState('4'))
	{
		camera.DecreaseNearPlane((offset + 10.0f) * delta);
	}
	// increase far plane
	if (GetAsyncKeyState('5'))
	{
		camera.IncreaseFarPlane((offset + 10.0f) * delta);
	}
	// decrease far plane
	if (GetAsyncKeyState('6'))
	{
		camera.DecreaseFarPlane((offset + 10.0f) * delta);
	}
	// waviness
	if (GetAsyncKeyState('F') & 0x1)
		wave = !wave;
	if (GetAsyncKeyState('R') & 0x1)
	{
		camera.SetPosition(0.0f, 0.0f, -20.0f);
		camera.SetProjection(90.0f, 1.0f, 0.1f, 500.0f);
	}
	camera.UpdateView();
}
