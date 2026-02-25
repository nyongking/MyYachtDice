#pragma once

namespace Render
{
	class ConstantBuffer
	{
	public:
		ConstantBuffer() = default;
		~ConstantBuffer() = default;

		bool Create(ID3D11Device* device, UINT byteWidth, D3D11_USAGE usage = D3D11_USAGE_DYNAMIC);

		bool Update(ID3D11DeviceContext* context, const void* data, UINT byteWidth, UINT offset = 0);

		void BindVS(ID3D11DeviceContext* context, UINT slot = 0) const;
		void BindPS(ID3D11DeviceContext* context, UINT slot = 0) const;

		ID3D11Buffer* Get() const { return m_buffer.Get(); }
		bool IsCreated() const { return m_buffer.Get() != nullptr; }
		UINT Size() const { return m_size; }

	private:
		static UINT Align16(UINT size) { return (size + 15u) & ~15u; }

	private:
		Render::RefCom<ID3D11Buffer> m_buffer;
		UINT m_size = 0;
		D3D11_USAGE m_usage = D3D11_USAGE_DEFAULT;
	};
}

