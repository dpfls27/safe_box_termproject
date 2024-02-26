# safe_box_termproject
마이크로프로세서 실습을 통한 금고 텀프로젝트 과제물이다.

###### **부저** - 연속된 음계 출력 시 각 음계간의 출력 사이 무음 시간이 있어 음계의 출력 횟수 파악이 가능해야한다. / 음계의 출력시 시간이 정해지지 않은 경우, 청각을 통하여 충분히 파악가능한 수준으로 설정해야한다. 


###### **서보모터** - 잠금 상태는 0도 이며 해제 상태는 90도이다. 


###### **키패드** - 키패드 상의 키는 누를 때 1회만 인식한다. 



**1. 키패드 인터페이스 구현하기**
   - 시스템 초기 동작 시 아무 출력 없을 것
   - 키패드 입력 시 부저를 통하여 소리를 출력할 것
   - 출력되는 음은 1: 4-도 2: 4-레 3: 4-미 4: 4-파 5:4-솔 6: 4-라 7: 4-시 8: 5-도 9: 5-레 이외: 5-미
   - 부저의 출력은 누름시 1회(50ms)진행, 키패드를 누르고 있는 경우 키패드에서 손을 뗄 때까지 소리를 지속하여 출력
   - If 키패드의 키가 2개 이상 눌리는 경우 먼저 입력된 키만 허용

**2. 초기 동작 구현하기**
   - 초기 상태에서는 부저를 통하여 4-솔 음을 1회/100ms 출력
   - RC 서보모터는 0도를
   - FND 및  LCD에 아무것도 출력하지 않음
  
**3. 비밀번호 일치에 따른 문 개방하기**
   - 키패드의 #버튼을 누르면 LCD에 비밀번호 입력을 위해 아래와 같은 상태가 표시됨

     
     PASSWORD **************

   - 키패드의 숫자 6-14 자리를 누르고 * 버튼을 눌러 비밀번호 일치 확인 (초기값은 1 2 3 4 5 7 8 9 0)


     이때, 키패드 눌림시 모든 키 값은 FND+LCD에 출력 후 사용자가 확인하도록 함 / 눌리는 키에 따라 FND 출력되는 형태는 좌로 쉬프트 되면서 출력

     
     PASSWORD
     1************* [LCD 출력- 1 누른경우]

     
     PASSWORD
     12************ [LCD 출력- 1,2 누른경우]

     
     ㅁㅁ12 [FND 출력 - 1,2 누른경우, ㅁ은 빈칸의미]

   - 비밀번호 일치 확인

     - 일치 시에는 부저로 4-도/4-미/4-솔/5-도 를 각각 500ms 씩 재생하고 서보모터를 움직여 잠금 해제(서보모터의 각도를 90도로)후 LCD에 아래와 같이 출력
    

       DOOR OPEN


     - 불일치 시에는 부저로 4-라/4-라/4-라 를 각각 500ms 씩 재생하고 LCD에 다음과 같이 출력한다. 


      PASSWORD error.......


     - 불일치 시 3초 후 초기 상태 즉 아무것도 출력하지 않는 상태로 돌아감
