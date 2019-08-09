#include "Graphics.h"

// shader includes
#include "VertexShader.csh"
#include "PixelShader.csh"
#include "../../My assets/test pyramid.h"
#include "../../My assets/StoneHenge.h"
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

			// TODO: Create new DirectX stuff here! (Buffers, Shaders, Layouts, Views, Textures, etc...)
			HRESULT hr = InitializeDevice();
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
				myContext->ClearDepthStencilView(myDepthStencilView, D3D11_CLEAR_DEPTH, 1, 0); // clear it to Z exponential Far.
				myDepthStencilView->Release();
			}

			// Set active target for drawing, all array based D3D11 functions should use a syntax similar to below
			ID3D11RenderTargetView* const targets[] = { myRenderTargetView };
			myContext->OMSetRenderTargets(1, targets, myDepthStencilView);

			// Clear the screen to green
			const float d_green[] = { 0.498f, 0.729f, 0, 1 }; // "DirectX Green"
			myContext->ClearRenderTargetView(myRenderTargetView, d_green);

			// TODO: Set your shaders, Update & Set your constant buffers, Attatch your vertex & index buffers, Set your InputLayout & Topology & Draw!
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

			// world
			ConstantBuffer cb;
			XMMATRIX temp = XMMatrixIdentity();
			temp = XMMatrixTranslation(0.0f, 0.0f, 0.5f);
			//temp = XMMatrixTranspose(temp);
			XMStoreFloat4x4(&cb.world, temp);
			// view
			temp = XMMatrixLookAtLH({ 2.0f, 1.0f, -3.0f }, { 0, 0, 0 }, { 0, 1, 0 });
			//temp = XMMatrixTranspose(temp);
			XMStoreFloat4x4(&cb.view, temp);
			// projection
			float ar = 0.0f;
			mySurface->GetAspectRatio(ar);
			temp = XMMatrixPerspectiveFovLH(XM_PIDIV2, ar, 0.1f, 10.0f);
			//temp = XMMatrixTranspose(temp);
			XMStoreFloat4x4(&cb.projection, temp);

			D3D11_MAPPED_SUBRESOURCE gpuBuffer;
			myContext->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
			//*((ConstantBuffer*)(gpuBuffer.pData)) = GCB;
			// another way
			memcpy(gpuBuffer.pData, &cb, sizeof(ConstantBuffer));
			myContext->Unmap(constantBuffer, 0);
			ID3D11Buffer* constants[] = { constantBuffer };
			myContext->VSSetConstantBuffers(0, 1, constants);

			myContext->PSSetShaderResources(0, 1, &shaderRV);
			myContext->PSSetSamplers(0, 1, &sampler);
			// draw
			//myContext->Draw(numVertices, 0);
			myContext->DrawIndexed(numIndicies, 0, 0);

			// Present Backbuffer using Swapchain object
			// Framerate is currently unlocked, we suggest "MSI Afterburner" to track your current FPS and memory usage.
			mySwapChain->Present(0, 0); // set first argument to 1 to enable vertical refresh sync with display

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
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL"  , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = myDevice->CreateInputLayout(layout, ARRAYSIZE(layout), VertexShader, sizeof(VertexShader), &vertexLayout);

	// fill vertices
	//numVertices = ARRAYSIZE(test_pyramid_data);
	//numIndicies = ARRAYSIZE(test_pyramid_indicies);
	numVertices = ARRAYSIZE(StoneHenge_data);
	numIndicies = ARRAYSIZE(StoneHenge_indicies);
	vertices = new Vertex[numVertices];
	//for (size_t i = 0; i < numVertices; i++)
	//{
	//	vertices[i].position.x = test_pyramid_data[i].pos[0];
	//	vertices[i].position.y = test_pyramid_data[i].pos[1];
	//	vertices[i].position.z = test_pyramid_data[i].pos[2];
	//	vertices[i].position.w = 1.0f;
	//	vertices[i].texture.x = test_pyramid_data[i].uvw[0];
	//	vertices[i].texture.y = test_pyramid_data[i].uvw[1];
	//	vertices[i].texture.z = 1.0f;
	//	vertices[i].texture.w = 1.0f;
	//	vertices[i].normal.x = test_pyramid_data[i].nrm[0];
	//	vertices[i].normal.y = test_pyramid_data[i].nrm[1];
	//	vertices[i].normal.z = test_pyramid_data[i].nrm[2];
	//	vertices[i].normal.w = 0.0f;
	//}
	for (size_t i = 0; i < numVertices; i++)
	{
		vertices[i].position.x = StoneHenge_data[i].pos[0] * 0.1f;
		vertices[i].position.y = StoneHenge_data[i].pos[1] * 0.1f;
		vertices[i].position.z = StoneHenge_data[i].pos[2] * 0.1f;
		vertices[i].position.w = 1.0f;

		vertices[i].texture.x = StoneHenge_data[i].uvw[0];
		vertices[i].texture.y = StoneHenge_data[i].uvw[1];

		vertices[i].normal.x = StoneHenge_data[i].nrm[0];
		vertices[i].normal.y = StoneHenge_data[i].nrm[1];
		vertices[i].normal.z = StoneHenge_data[i].nrm[2];
		vertices[i].normal.w = 0.0f;
	}
	indicies = (void*)StoneHenge_indicies;
	// describe vertex data and create vertex buffer
	hr = CreateBuffer(myDevice, &vertexBuffer, D3D11_BIND_VERTEX_BUFFER, sizeof(Vertex) * numVertices, vertices);
	// describe index data and create index buffer
	hr = CreateBuffer(myDevice, &indexBuffer, D3D11_BIND_INDEX_BUFFER, sizeof(unsigned int) * numIndicies, indicies);
	// describe constant variables and create constant buffer
	hr = CreateBuffer(myDevice, &constantBuffer, D3D11_BIND_CONSTANT_BUFFER, sizeof(ConstantBuffer), nullptr);
	// load texture
	hr = CreateDDSTextureFromFile(myDevice, L"../../My assets/StoneHenge.dds", nullptr, &shaderRV);
	// create sample
	D3D11_SAMPLER_DESC sampDesc = {};
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
	vertexBuffer->Release();
	indexBuffer->Release();
	vertexLayout->Release();
	vertexShader->Release();
	pixelShader->Release();
	delete[] vertices;
	vertices = nullptr;
}

HRESULT Graphics::CreateBuffer(ID3D11Device* device, ID3D11Buffer** buffer, UINT bindFlag, UINT byteWidth, const void* pSysMem)
{
	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA subRsrcData;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&subRsrcData, sizeof(D3D11_SUBRESOURCE_DATA));
	bufferDesc.ByteWidth = byteWidth;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	switch (bindFlag)
	{
	case D3D11_BIND_VERTEX_BUFFER:
		bufferDesc.BindFlags = bindFlag;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		subRsrcData.pSysMem = pSysMem;
		device->CreateBuffer(&bufferDesc, &subRsrcData, buffer);
		return S_OK;
	case D3D11_BIND_INDEX_BUFFER:
		bufferDesc.BindFlags = bindFlag;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		subRsrcData.pSysMem = pSysMem;
		device->CreateBuffer(&bufferDesc, &subRsrcData, buffer);
		return S_OK;
	case D3D11_BIND_CONSTANT_BUFFER:
		bufferDesc.BindFlags = bindFlag;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		device->CreateBuffer(&bufferDesc, nullptr, buffer);
		return S_OK;
	default:
		return S_FALSE;
	}
}
