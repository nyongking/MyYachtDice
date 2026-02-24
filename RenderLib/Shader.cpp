#include "RenderPch.h"
#include "Shader.h"

namespace Render
{
	bool Shader::ReflectShader()
	{
		if (nullptr == m_reflection)
			return false;

		D3D11_SHADER_DESC shaderDesc;
		m_reflection->GetDesc(&shaderDesc);

        // cbuffers
        for (UINT i = 0; i < shaderDesc.ConstantBuffers; ++i) 
        {
            ID3D11ShaderReflectionConstantBuffer* pCB = m_reflection->GetConstantBufferByIndex(i);
            D3D11_SHADER_BUFFER_DESC cbDesc;
            pCB->GetDesc(&cbDesc);

            ConstantBufferInfo info;
            info.name = cbDesc.Name;
            info.size = cbDesc.Size;

            // 변수들 상세 정보 파싱
            for (UINT j = 0; j < cbDesc.Variables; ++j) 
            {
                ID3D11ShaderReflectionVariable* pVar = pCB->GetVariableByIndex(j);
                D3D11_SHADER_VARIABLE_DESC varDesc;
                pVar->GetDesc(&varDesc);

                info.variables.push_back({ varDesc.Name, varDesc.StartOffset, varDesc.Size });
            }

            m_cbuffers.push_back(info);
        }

        // resources (texture, samplers)
        for (UINT i = 0; i < shaderDesc.BoundResources; ++i) 
        {
            D3D11_SHADER_INPUT_BIND_DESC resDesc;
            m_reflection->GetResourceBindingDesc(i, &resDesc);

            ResourceBindInfo resInfo;
            resInfo.name = resDesc.Name;
            resInfo.type = resDesc.Type;
            resInfo.slot = resDesc.BindPoint; // 예: t0에서 0

            m_resources.push_back(resInfo);
        }

		return true;
	}
}