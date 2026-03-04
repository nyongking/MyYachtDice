#include "RenderPch.h"
#include "Material.h"

namespace Render
{
	Material::Material(const Material& rhs)
		: m_shaderGroup(rhs.m_shaderGroup)
		, m_constantBufferParameters(rhs.m_constantBufferParameters)
		, m_constantBufferSlots(rhs.m_constantBufferSlots)
	{
	}

	Material::~Material()
	{
		for (auto& param : m_constantBufferParameters)
		{
			param.Release();
		}

		m_constantBufferParameters.clear();
	}

	bool Material::BindMaterial(ID3D11DeviceContext* pContext)
	{
		if (nullptr == pContext)
			return false;

		for (auto& param : m_constantBufferParameters)
			param.Update(pContext);

		for (auto& slot : m_constantBufferSlots)
			slot.BindBufferToSlot(pContext);

		return true;
	}


	int Material::RegisterConstantBufferSlot(class ConstantBuffer* pBuffer)
	{
		if (nullptr == pBuffer)
			return -1;

		m_constantBufferSlots.push_back(ConstantBufferSlot(pBuffer));

		return static_cast<int>(m_constantBufferSlots.size() - 1);
	}

	bool Material::ConnectConstantBufferToVSSlot(const std::string& name, int index, ShaderGroup* pShaderGroup)
	{
		if (index < 0 || index >= static_cast<int>(m_constantBufferSlots.size()))
			return false;

		ConstantBufferSlot& dat = m_constantBufferSlots[index];

		return dat.ConnectVSSlot(name, pShaderGroup);
	}

	bool Material::ConnectConstantBufferToPSSlot(const std::string& name, int index, ShaderGroup* pShaderGroup)
	{
		if (index < 0 || index >= static_cast<int>(m_constantBufferSlots.size()))
			return false;

		ConstantBufferSlot& dat = m_constantBufferSlots[index];

		return dat.ConnectPSSlot(name, pShaderGroup);
	}
}

