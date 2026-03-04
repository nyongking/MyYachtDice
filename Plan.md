# 260305

1. 현재 Material의 Release 과정 없애야 한다. + ConstantBufferManager는 unique_ptr로 갖도록 한다.
2. Clone시에 ConstantBufferParameter->m_pData를 교체하는 작업 + Clone 시에 Initialize 과정 삭제
3. ViewProjManager 삭제 예정
4. DefaultColorMaterial을 Client/Tool에 가져올 것
5. Assimp를 통해서 모델 로딩 -> 일부 Parsing까지 거치기

