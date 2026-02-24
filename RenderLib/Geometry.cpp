#include "RenderPch.h"
#include "Geometry.h"

namespace Render
{
	bool Geometry::BindAndDraw(ID3D11DeviceContext* pContext)
	{
		if (nullptr == pContext || nullptr == m_vb || nullptr == m_ib)
			return false;

		UINT32 offset = 0;
		pContext->IASetVertexBuffers(0, 1, m_vb.GetAddressOf(), &m_vertexStride, &offset);
		pContext->IASetPrimitiveTopology(m_topology);
		pContext->IASetIndexBuffer(m_ib.Get(), m_indexFormat, 0);

		// draw
		pContext->DrawIndexed(m_indexCount, 0, 0);

		return true;
	}

	bool Geometry::CreateIB(ID3D11Device* pDevice, std::vector<unsigned __int16> indices)
	{
		D3D11_BUFFER_DESC bufferDesc;
		D3D11_SUBRESOURCE_DATA subDesc;

		memset(&bufferDesc, 0, sizeof(bufferDesc));
		memset(&subDesc, 0, sizeof(subDesc));

		bufferDesc.ByteWidth = m_indexStride * static_cast<UINT>(indices.size());
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.StructureByteStride = 0;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;

		subDesc.pSysMem = indices.data();

		if (FAILED(pDevice->CreateBuffer(&bufferDesc, &subDesc, m_ib.GetAddressOf())))
		{
			return false;
		}

		return true;
	}
}

