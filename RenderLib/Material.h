#pragma once
#include "ShaderGroup.h"
#include "Materialparameter.h"
#include "ConstantBufferManager.h"

namespace Render
{
	class Material
	{
	public:
		Material() = default;
		virtual ~Material() = default;

	public:
		virtual bool Initialize(const ShaderGroup* shaderGroup) abstract;

	protected:
		template<typename T>
		bool		 RegisterConstantBuffer(const std::string& name, T* const pData, bool isDynamic = true);

	private:
		std::vector<ConstantBufferParameter> m_constantBufferParameters;
	};

	template<typename T>
	inline bool Material::RegisterConstantBuffer(const std::string& name, T* const pData, bool isDynamic)
	{
		std::shared_ptr<ConstantBuffer> buffer = ConstantBufferManager::GetInstance().AddOrGetConstantBuffer(name, sizeof(T), pData, isDynamic);

		if (nullptr == buffer)
			return false;

		m_constantBufferParameters.push_back()

		return false;
	}
}

