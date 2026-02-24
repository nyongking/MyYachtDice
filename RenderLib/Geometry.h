#pragma once

namespace Render
{
	class Geometry abstract
	{
	public:
		Geometry() = default;
		virtual ~Geometry() = default;

	public:
		virtual bool	DefaultCreateBuffers(ID3D11Device* pDevice) {}
		void			BindAndDraw(ID3D11DeviceContext* pContext);


	protected:
		template<typename BufferT>
		bool		CreateVB(ID3D11Device* pDevice, const std::vector<BufferT>& vertices);
		bool		CreateIB(ID3D11Device* pDevice, std::vector<unsigned __int16> indices);

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

