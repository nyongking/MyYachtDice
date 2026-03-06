# 260305

1. 현재 Material의 Release 과정 없애야 한다. + ConstantBufferManager는 unique_ptr로 갖도록 한다. (해결 완료)
2. Clone시에 ConstantBufferParameter->m_pData를 교체하는 작업 + Clone 시에 Initialize 과정 삭제. (해결 완료)
3. ViewProjManager 삭제 예정 (해결 완료)
4. DefaultColorMaterial을 Client/Tool에 가져올 것 (해결 완료)
5. Assimp를 통해서 모델 로딩 -> 일부 Parsing까지 거치기

# 260306

1. `Render::ConstantBufferManager` -> `Engine::ConstantBufferManager`으로 위치 변경 방안 및 `Material`과 `ConstantBufferManager` 의 의존성 해결, `ConstantBuffer` 연결 방식 변경

   (변경) 이에 대해서 `ConstantBufferManager` 삭제, `ConstantBuffer`는 `Material` Clone 시에 복사해서 사용한다. (해결 완료)
2. `Render::Geometry` 를 상속 혹은 확장하는 형태의 `Render::Model` 클래스 방식 설정

   (변경) `Engine::Model` 클래스를 통해서 여러개의 `Render::Material` + `Render::Geometry` 를 사용하는 형태 (해결 완료)
3. `Light` 요소 도입

# 260307

1. `Engine::MaterialManager`가 `Engine::ShaderManager`에 의존하는 상황, 이에 대한 분리 필요 혹은 `Engine::MaterialManager`의 변경 혹은 삭제..

2. `Light` 도입 및 MRT 렌더링 + `Render::RenderPass` 및 `Render::RenderCommand`의 다양화
