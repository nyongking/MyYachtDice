#pragma once
#include "ShaderGroup.h"
#include "Materialparameter.h"
#include "MaterialSlot.h"
#include "ConstantBufferManager.h"

namespace Render
{
	class Material
	{
	public:
		Material() = default;
		Material(const Material& rhs);
		virtual ~Material();

	public:
		virtual bool Initialize(ShaderGroup* shaderGroup) abstract;
		virtual std::unique_ptr<Material> Clone() const = 0;

		bool		 BindMaterial(ID3D11DeviceContext* pContext);

		ShaderGroup* GetShaderGroup() const { return m_shaderGroup; }

		virtual void SetWorld(const float4x4& w)     {}
		virtual void SetViewProj(const float4x4& vp) {}

	protected:
		template<typename T>
		int			RegisterConstantBuffer(const std::string& name, T* const pData, bool isDynamic = true);
		int			RegisterConstantBufferSlot(ConstantBuffer* pBuffer);
		bool		ConnectConstantBufferToVSSlot(const std::string& name, int index, class ShaderGroup* pShaderGroup);
		bool		ConnectConstantBufferToPSSlot(const std::string& name, int index, class ShaderGroup* pShaderGroup);

		template<typename T>
		bool		AutomaticRegisterVS(const std::string& name, const std::string& slotname, T* const pData, class ShaderGroup* pShaderGroup, bool isDynamic = true);

		template<typename T>
		bool		AutomaticRegisterPS(const std::string& name, const std::string& slotname, T* const pData, class ShaderGroup* pShaderGroup, bool isDynamic = true);


	protected:
		ShaderGroup* m_shaderGroup = nullptr;

	private:
		std::vector<ConstantBufferParameter> m_constantBufferParameters;
		std::vector<ConstantBufferSlot>		 m_constantBufferSlots;
	};

	template<typename T>
	inline int Material::RegisterConstantBuffer(const std::string& name, T* const pData, bool isDynamic)
	{
		if (nullptr == pData)
			return -1;

		std::shared_ptr<ConstantBuffer> buffer = ConstantBufferManager::GetInstance().AddOrGetConstantBuffer(name, sizeof(T), pData, isDynamic);

		if (nullptr == buffer)
			return -1;

		m_constantBufferParameters.push_back(ConstantBufferParameter{pData, std::move(buffer)});

		return static_cast<int>(m_constantBufferParameters.size() - 1);
	}

	template<typename T>
	inline bool Material::AutomaticRegisterVS(const std::string& name, const std::string& slotname, T* const pData, ShaderGroup* pShaderGroup, bool isDynamic)
	{
		int index = RegisterConstantBuffer(name, pData, isDynamic);

		if (-1 == index)
			return false;

		int slotIndex = RegisterConstantBufferSlot(m_constantBufferParameters[index].Buffer());

		ConnectConstantBufferToVSSlot(slotname, slotIndex, pShaderGroup);

		return true;
	}

	template<typename T>
	inline bool Material::AutomaticRegisterPS(const std::string& name, const std::string& slotname, T* const pData, ShaderGroup* pShaderGroup, bool isDynamic)
	{
		int index = RegisterConstantBuffer(name, pData, isDynamic);

		if (-1 == index)
			return false;

		int slotIndex = RegisterConstantBufferSlot(m_constantBufferParameters[index].Buffer());

		ConnectConstantBufferToPSSlot(slotname, slotIndex, pShaderGroup);

		return true;
	}
}

