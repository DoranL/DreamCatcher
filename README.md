# DreamCatcher



![제목을-입력해주세요_](https://github.com/DoranL/DreamCatcher/assets/91326457/235e1dc4-b872-4fda-8f6d-798caeaf6a6d)

유투브 영상 시청 : https://youtu.be/vKTOGqLspRM

프로젝트 기능 

+ 시작 화면, 스테이지(1, 2, 3), 엔딩, 엔딩 크레딧 총 6개의 맵을 사용하였고 각기 다른 스킬 또는 역할을 하는 13명의 캐릭터가 등장

+ 게임 시작 화면에서 게임 시작, 게임 이어 하기, 게임 종료 구현

+ 게임 편리성을 위한 각종 UI 구현

 -> 메인 UI(체력, 스태미나, 스킬, 레벨 및 경험치)

 -> 조작키, 대화창, 일시정지, 힌트, 엔딩 크레딧, 엔딩 선택, You Died, 타겟팅 UI 제공 

+ NPC 및 적과의 대화가 존재하며 선택한 대화에 따른 다른 스토리 전개 제공(E 키를 통해 

  대화가 가능하다.)

+ 보스 Strafing 기능 구현

+ Save & Load 시스템 구현

+ 플레이어 다양한 기능 제공

 -> W, A, S, D 키를 사용한 이동 

 -> Shift 키를 사용한 달리기 및 spacebar를 사용한 점프 기능 구현

 -> Alt 키를 통해 대시 가능(회피)

 -> 숫자키 1, 2, 3을 통한 스킬 구현

 -> 숫자키 4번을 사용하여 체력 회복 가능

 -> Q 키와 ESC 키를 사용하여 일시 정지 활성화 End 키를 사용하여 긴급 탈출 기능 구현

 -> 왼쪽 마우스를 사용하여 적 공격 패링 가능 

 -> 왼쪽 마우스와 G 키를 통해 오브젝트 줍기 가능 

 -> 마우스 스크롤을 통해 카메라 줌인

+ 각종 이벤트 구현

 -> 첫 번째 스테이지에서는 빠른 시간 내에 벽을 오르고 최대한 빨리 적을 불러들이는 정찰병을 처치하여 적은 숫자의 적을 생성해야 한다. (벽 오르기 기능 구현)

 -> 두 번째 스테이지에서는 해당 스테이지의 보스를 구출하는 퀴즈 요소 추가(결속을 해제하고 

​    신을 탈출시켜야 한다.) - 플러그인 사용

 -> 세 번째 스테이지에서는 계속해서 부활하는 캐릭터를 처치하기 위해 그들의 왕을 찾아 처치 해야 한다. 그 이후 보스에게 갈 수 있으며 보스 처치 후 모은 4개의 오르도 각인을 석상에  꽂아 새로운 통치자를 계승해야 한다.

+ 레벨 시스템은 적 처치 시 경험치를 얻을 수 있고 레벨에 따라 스킬이 해제

  레벨 1이면 스킬 1번 레벨이 3이면 스킬 1, 2, 3번 사용이 가능(스킬 쿨타임 구현)

+ 체력 시스템 및 스태미나 시스템 구현. 달릴 시 스태미나가 감소

+ 스폰 체크포인트 도달 시 죽었을 경우 해당 스폰 포인트에서 부활

+ 시퀀스 제작

 -> 각 스테이지 마다 게임 스토리의 이해를 돕기 위해 unreal engine의 레벨 시퀀스를 통해 영상을 제작하여 삽입
 
