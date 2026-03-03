#pragma once

namespace Render
{
	class ConstantBuffer
	{
	public:
		ConstantBuffer() = default;
		ConstantBuffer(uint64_t id);
		~ConstantBuffer() = default;

		bool Create(ID3D11Device* device, uint32_t byteWidth, const void* pInitialData, D3D11_USAGE usage = D3D11_USAGE_DYNAMIC);
		bool Release();

		bool Update(ID3D11DeviceContext* context, const void* data, uint32_t byteWidth, uint32_t offset = 0);

		void BindVS(ID3D11DeviceContext* context, uint32_t slot = 0) const;
		void BindPS(ID3D11DeviceContext* context, uint32_t slot = 0) const;

		ID3D11Buffer* Get() const { return m_buffer.Get(); }
		bool IsCreated() const { return m_buffer.Get() != nullptr; }
		uint32_t Size() const { return m_size; }

	private:
		static UINT Align16(UINT size) { return (size + 15u) & ~15u; }

	private:
		Render::RefCom<ID3D11Buffer> m_buffer;
		
		const uint64_t m_id = 0;
		uint32_t m_size = 0;
		D3D11_USAGE m_usage = D3D11_USAGE_DEFAULT;
	};
}

