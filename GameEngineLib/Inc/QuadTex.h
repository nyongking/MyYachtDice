#pragma once
#include "Geometry.h"

namespace Render
{
	class QuadTex : public Geometry
	{
	public:
		QuadTex() = default;
		virtual ~QuadTex() = default;

	public:
		virtual bool DefaultCreateBuffers(ID3D11Device* pDevice) override;
	};


}

