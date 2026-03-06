# 모델

## 개요

assimp로 읽어들인 모델(.fbx, .obj 등 assimp로 읽을 수 있는 타입)을 나만의 모델 확장자(.mymesh, .myanimmesh, .myanim, mymat) 형태로 저장하고 사용할 수 있도록 한다.

Tool 프로젝트에서 Assimp로 모델을 읽어 나만의 확장자 형태로 저장, Client/Tool에서 이를 읽어서 쓸 수 있도록 한다.



## 확장자 내용

`.mymesh` : 애니메이션이 없는 모델들의 정점 데이터를 이 형태로 저장한다 (예상 정점 타입 : Position, UV, Normal, Tangent )

`.myanimmesh` : 애니메이션 정보가 있는 모델들의 정점 데이터를 이러한 형태로 저장한다 (예상 정점 타입 : Position, UV, Normal, Tangent, BlendIndex, BlendWeight + Bone 정보)

`.myanim` : 애니메이션 정보를 담는다. (저장 데이터 미정)

`.mymat`: material 정보를 담는다 (diffuse, normal 과 같은 내용)



## 작업



## Tool

Tool에서는 Assimp로 읽은 데이터를 나만의 형태로 저장하거나, 이 저장된 형태를 읽을 수 있도록 해야한다.

Tool에서는 모델 내용을 수정하거나 확인할 수 있어야 한다.



## Client

Client는 Assimp 라이브러리를 사용하지 않는다. 오로지 나만의 타입을 이용하여 로드하고 정점 데이터 및 material로 바꿔서 사용해야 한다.



## RenderLib 및 GameEngineLib

RenderLib/GameEngineLib은 Geometry의 확장버전 혹은 상속을 통해서 static mesh, anim mesh을 다룰 수 있도록 해야 한다. (세부적인 내용은 계속해서 발전해나갈 예정)

GameEngineLib은 이를 비동기/동기 로드 형태를 지원할 수 있어야 한다.





## 구현

애니메이션이 있는 모델은 추후 구현한다. 먼저 애니메이션이 없는(스태틱) 모델에 대해서 차례로 구현한다.

1. RenderLib에서 Geometry의 확장 버전을 먼저 구현한다.
2. 일단 Tool에서 Assimp를 통해 특정 .fbx 로드가 되는지 확인하고, 정점 정보와 material 정보를 읽어들인다.
3. 이를 나만의 확장자의 데이터로 저장한다.