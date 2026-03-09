#pragma once
#include "Geometry.h"

namespace GameEngine
{
	class Plane : public Render::Geometry
	{
	public:
		Plane()  = default;
		~Plane() = default;

	public:
		bool DefaultCreateBuffers(ID3D11Device* pDevice) override;
	};
}
