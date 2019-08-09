#include "Graphics.h"

// shader includes
#include "VertexShader.csh"
#include "PixelShader.csh"
#include "../../My assets/test pyramid.h"

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

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	D3D11_SUBRESOURCE_DATA subRsrcData;
	ZeroMemory(&subRsrcData, sizeof(D3D11_SUBRESOURCE_DATA));

	// fill vertices
	numVertices = ARRAYSIZE(test_pyramid_data);
	numIndicies = ARRAYSIZE(test_pyramid_indicies);
	vertices = new Vertex[numVertices];
	for (size_t i = 0; i < numVertices; i++)
	{
		vertices[i].position.x = test_pyramid_data[i].pos[0];
		vertices[i].position.y = test_pyramid_data[i].pos[1];
		vertices[i].position.z = test_pyramid_data[i].pos[2];
		vertices[i].position.w = 1.0f;

		vertices[i].texture.x = test_pyramid_data[i].uvw[0];
		vertices[i].texture.y = test_pyramid_data[i].uvw[1];
		vertices[i].texture.z = 1.0f;
		vertices[i].texture.w = 1.0f;

		vertices[i].normal.x = test_pyramid_data[i].nrm[0];
		vertices[i].normal.y = test_pyramid_data[i].nrm[1];
		vertices[i].normal.z = test_pyramid_data[i].nrm[2];
		vertices[i].normal.w = 0.0f;
	}
	// describe vertex data and create vertex buffer
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(Vertex) * numVertices;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	subRsrcData.pSysMem = vertices;
	hr = myDevice->CreateBuffer(&bufferDesc, &subRsrcData, &vertexBuffer);
	// describe index data and create index buffer
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(unsigned int) * numIndicies;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	subRsrcData.pSysMem = test_pyramid_indicies;
	hr = myDevice->CreateBuffer(&bufferDesc, &subRsrcData, &indexBuffer);

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
