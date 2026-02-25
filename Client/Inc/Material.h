#pragma once

namespace Render
{
	class Material
	{
	public:
		Material() = default;
		virtual ~Material() = default;

	public:
		virtual bool Initialize() abstract;

	};
}

