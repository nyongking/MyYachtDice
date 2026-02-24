#include "RenderPch.h"
#include "VertexShader.h"

namespace Render
{
    DXGI_FORMAT DetermineFormat(D3D_REGISTER_COMPONENT_TYPE componentType, BYTE mask)
    {
        int componentCount = 0;
        if (mask & 1) ++componentCount;
        if (mask & 2) ++componentCount;
        if (mask & 4) ++componentCount;
        if (mask & 8) ++componentCount;

        if (componentType == D3D_REGISTER_COMPONENT_UINT32)
        {
            if (1 == componentCount) return DXGI_FORMAT_R32_UINT;
            if (2 == componentCount) return DXGI_FORMAT_R32G32_UINT;
            if (3 == componentCount) return DXGI_FORMAT_R32G32B32_UINT;
            if (4 == componentCount) return DXGI_FORMAT_R32G32B32A32_UINT;
        }
        else if (componentType == D3D_REGISTER_COMPONENT_SINT32)
        {
            if (1 == componentCount) return DXGI_FORMAT_R32_SINT;
            if (2 == componentCount) return DXGI_FORMAT_R32G32_SINT;
            if (3 == componentCount) return DXGI_FORMAT_R32G32B32_SINT;
            if (4 == componentCount) return DXGI_FORMAT_R32G32B32A32_SINT;
        }
        else if (componentType == D3D_REGISTER_COMPONENT_FLOAT32)
        {
            if (1 == componentCount) return DXGI_FORMAT_R32_FLOAT;
            if (2 == componentCount) return DXGI_FORMAT_R32G32_FLOAT;
            if (3 == componentCount) return DXGI_FORMAT_R32G32B32_FLOAT;
            if (4 == componentCount) return DXGI_FORMAT_R32G32B32A32_FLOAT;
        }
    }

    bool VertexShader::LoadFromFile(const std::wstring& path, const std::string& entry, const std::string& profile)
    {
        UINT Flag = 0;

#ifdef _DEBUG
        Flag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        Flag = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

        // compile
        if (FAILED(D3DCompileFromFile(
            path.c_str(),
            nullptr,
            nullptr,
            entry.c_str(),
            profile.c_str(),
            Flag,
            0,
            m_shaderBlob.GetAddressOf(),
            m_errorBlob.GetAddressOf()
        )))
        {
            return false;
        }

        // reflection
        if (FAILED(D3DReflect(
            m_shaderBlob->GetBufferPointer(),
            m_shaderBlob->GetBufferSize(),
            IID_ID3D11ShaderReflection, (void**)m_reflection.GetAddressOf())))
        {
#ifdef _DEBUG
            OutputDebugStringA("VertexShader Reflect Failed");
#endif
            return false;
        }

        // super::
        if (!ReflectShader())
        {
            return false;
        }
     
        return true;
    }

    bool VertexShader::CreateFromShaderBlob(ID3D11Device* pDevice)
    {
        if (nullptr == m_shaderBlob)
        {
#ifdef _DEBUG
            OutputDebugStringA("Invalid ShaderBlob");
#endif
            return false;
        }

        if (nullptr == m_reflection)
        {
#ifdef _DEBUG
            OutputDebugStringA("Invalid Reflection");
#endif
            return false;
        }

        // vertex shader
        if (FAILED(pDevice->CreateVertexShader(
            m_shaderBlob->GetBufferPointer(),
            m_shaderBlob->GetBufferSize(),
            nullptr,
            m_vertexShader.GetAddressOf()
        )))
        {
#ifdef _DEBUG
            OutputDebugStringA("CreateVertexShader Failed");
#endif
            return false;
        }


        D3D11_SHADER_DESC shaderDesc;
        m_reflection->GetDesc(&shaderDesc);

        std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDescs;

        for (UINT i = 0; i < shaderDesc.InputParameters; ++i)
        {
            D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
            m_reflection->GetInputParameterDesc(i, &paramDesc);

            D3D11_INPUT_ELEMENT_DESC elementDesc;
            elementDesc.SemanticIndex = paramDesc.SemanticIndex;
            elementDesc.SemanticName = paramDesc.SemanticName;

            elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
            if (0 == _stricmp(elementDesc.SemanticName, "SV_InstanceID"))
            {
                continue;
            }
            else if (0 == strncmp(elementDesc.SemanticName, "INST_", 5))
            {
                elementDesc.InputSlot = 1;
                elementDesc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
                elementDesc.InstanceDataStepRate = 1;
            }            
            else
            {
                elementDesc.InputSlot = 0;
                elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                elementDesc.InstanceDataStepRate = 0;
            }


            elementDesc.Format = DetermineFormat(paramDesc.ComponentType, paramDesc.Mask);

            inputElementDescs.push_back(elementDesc);
        }


        // input layout
        if (pDevice->CreateInputLayout(inputElementDescs.data(), static_cast<UINT>(inputElementDescs.size()),
            m_shaderBlob->GetBufferPointer(),
            m_shaderBlob->GetBufferSize(),
            m_InputLayout.GetAddressOf()))
        {
#ifdef _DEBUG
            OutputDebugStringA("CreateInputLayout Failed");
#endif
            return false;
        }

        m_shaderBlob.Reset();
        m_errorBlob.Reset();
        m_reflection.Reset();

        m_isCreated = true;

        return true;
    }

}
