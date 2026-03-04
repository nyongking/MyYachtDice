#pragma once

namespace GameEngine
{
	typedef struct POS
	{
		DirectX::XMFLOAT3		position;

		static const unsigned int			sNumElements = 1;
		static const D3D11_INPUT_ELEMENT_DESC sElements[sNumElements];
	}VTXPOS;

	typedef struct POSCOL
	{
		DirectX::XMFLOAT3		position;
		DirectX::XMFLOAT4		color;

		static const unsigned int			sNumElements = 2;
		static const D3D11_INPUT_ELEMENT_DESC sElements[sNumElements];
	}VTXPOSCOL;

	typedef struct POSTEX
	{
		DirectX::XMFLOAT3		position;
		DirectX::XMFLOAT2		texcoord;

		static const unsigned int			sNumElements = 2;
		static const D3D11_INPUT_ELEMENT_DESC sElements[sNumElements];
	}VTXPOSTEX;
}

