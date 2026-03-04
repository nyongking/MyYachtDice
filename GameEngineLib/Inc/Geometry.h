#pragma once

namespace Render
{
	class Geometry abstract
	{
	public:
		Geometry() = default;
		virtual ~Geometry() = default;

	public:
		virtual bool	DefaultCreateBuffers(ID3D11Device* pDevice) { return true; }
		bool			BindAndDraw(ID3D11DeviceContext* pContext);


	protected:
		template<typename BufferT>
		bool		CreateVB(ID3D11Device* pDevice, const std::vector<BufferT>& vertices);
		template<typename IndexT>
	bool		CreateIB(ID3D11Device* pDevice, const std::vector<IndexT>& indices)
	{
		static_assert(
			std::is_same_v<IndexT, uint16_t> || std::is_same_v<IndexT, uint32_t>,
			"Index type must be uint16_t or uint32_t");

		m_indexStride = static_cast<unsigned int>(sizeof(IndexT));
		m_indexCount  = static_cast<unsigned int>(indices.size());
		m_indexFormat = std::is_same_v<IndexT, uint16_t>
						  ? DXGI_FORMAT_R16_UINT
						  : DXGI_FORMAT_R32_UINT;

		D3D11_BUFFER_DESC   bd  = {};
		D3D11_SUBRESOURCE_DATA sd = {};
		bd.ByteWidth = m_indexStride * m_indexCount;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.Usage     = D3D11_USAGE_DEFAULT;
		sd.pSysMem   = indices.data();

		return SUCCEEDED(pDevice->CreateBuffer(&bd, &sd, m_ib.GetAddressOf()));
	}

	protected:
		RefCom<ID3D11Buffer> m_vb;
		RefCom<ID3D11Buffer> m_ib;
		unsigned int		 m_vertexStride;
		unsigned int		 m_vertexCount;
		unsigned int		 m_indexStride;
		unsigned int		 m_indexCount;
		DXGI_FORMAT			 m_indexFormat;
		D3D11_PRIMITIVE_TOPOLOGY m_topology;

	};

	template<typename BufferT>
	inline bool Geometry::CreateVB(ID3D11Device* pDevice, const std::vector<BufferT>& vertices)
	{
		D3D11_BUFFER_DESC bufferDesc;
		D3D11_SUBRESOURCE_DATA subDesc;

		memset(&bufferDesc, 0, sizeof(bufferDesc));
		memset(&subDesc, 0, sizeof(subDesc));

		bufferDesc.ByteWidth = m_vertexStride * m_vertexCount;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.StructureByteStride = m_vertexStride;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;

		subDesc.pSysMem = vertices.data();

		if (FAILED(pDevice->CreateBuffer(&bufferDesc, &subDesc, m_vb.GetAddressOf())))
		{
			return false;
		}

		return true;
	}
}

