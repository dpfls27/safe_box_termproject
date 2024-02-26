/*
 * term_project_2023_jyr.c
 *
 * Created: 2023-06-21 오후 4:43:32
 * Author : USER
 */ 

#define F_CPU 14745600UL

#include "LCD.h"
#include "COMPONENT.h"
#include "OUTPUT.h"

unsigned char master_key[11] = {'2','0','2','1','1','4','8','0','3','2'};

int current_mode = init_mode; // 현재 모드는 첫 동작 기준 초기모드로 초기화

unsigned int timer_count = 0; // 초 세는 변수
unsigned char open_star = 0; // door open상태 & *이 들어오면 1 / 아니면 0
unsigned int count = 0; //비밀번호 틀리는 횟수 체크

unsigned int dual_count = 0; // *과 # 을 누를때
int is_safe_mode = 0; //세이프모드 ON OFF
int is_dual = 0; //* # 같이 누르는지 아닌지


unsigned char password_secret[15] = {0,}; //비밀번호 받는 배열
unsigned char init_password[20] = {'1','2','3','4','5','6','7','8','9','0',};

unsigned char num_data[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xd8, 0x80, 0x90}; // 7-segment의 숫자 0~9까지 표현
unsigned int Port_fnd[] = {0x1f, 0x2f, 0x4f, 0x8f}; // PORTE.4 5 6 7 전환
	
// 초기값 설정 함수
void port_init() {
	//입력
	DDRC = 0xF0; // keypad
	
	DDRE = 0xF0; // fnd 출력자리(4~7)
	DDRF = 0xff; // fnd숫자실행 
	
	DDRG = 0x17; // buzzer(4) / LCD(0~2)
	DDRA = 0xFF; // lcd(0~7)
	
	DDRB = 0xFF; // 0111 1111 DDRB = 0xFF; 
	
	DDRD = 0x00; // 외부인터럽트 
	

}

// 초기값 설정 함수
void init_interface() {
	// FND
	no_fnd_disp();
	
	// LCD
	PORTA = 0x00; // 입력
	LCD_Init();
	
	// Buzzer
	PORTG = 0x10; // 0001 0000
	
	// Motor
	Init_Timer1();

	// Timer
	Init_Timer2();
	
	EICRA &= ~(3<<ISC00); //INT0 Falling Edge 감지설정
	EICRA |= (1<<ISC01) | (0<<ISC00);
	// EICRA = 0x02; // 0000 0010
	
	EIMSK |= (1<<INT0); //INT0 활성화
	
	// 전체 인터립트 허가
	sei();
}

// 모터 제어를 위한 함수
void Init_Timer1(void)
{
	// 고속 PWM , TOP = ICR1;, 1분주
	TCCR1A |=  (1<<COM1A1) | (0<<COM1A0) | (1<<WGM11);
	TCCR1B |= (1<<WGM13) | (1<<WGM12) | (0<<CS11) | (1<<CS10);
	ICR1=  294912;     //TOP  ICR1 = 14745600(F_CPU)/1(분주)/50(Hz) - 1 즉, 294912
	OCR1A= 22117;     //0도  20ms(50Hz기준): 1.5ms = 294912 : OCR1A 즉 22117
	// 294912 22117(0도) 29491(90도)
}

// 타이머/카운터2 타이머 초기화
void Init_Timer2(){
	TCCR2 = 0x00;
	TCCR2 |= (1<<WGM21) | (1 << COM20); // CTC 모드
	OCR2 = 100;
	TIMSK |= 1 << OCIE2;
	TCCR2 |= 1<< CS21;
}

//fnd 디스플레이 초기값
void no_fnd_disp() {
	PORTE = 0xff; // 자릿수
	PORTF = 0xff; // 입력
}

// 열 함수에 쓰이는 딜레이 함수
void myDelay_us(unsigned int delay){
	int i;
	for(i=0; i<delay; i++){
		_delay_us(1);
	}
}

// 열 만드는 함수
void Set_Row(unsigned char auto_row){
	PORTC = 0xF0; // row 출력 1 = 아무것도 선택 안한거다, col 입력 0 = 아무것도 선택 안한거다 (PORTC = 0xF0 뒤에는 출력으로 초기화, 앞에는 입력으로 초기)
	PORTC &= ~(1<<(4+auto_row));
	_delay_us(1);
}

// 음계 출력하는 함수
void SSound(int time, unsigned int during){
	int i, tim;
	tim = during / time;
	for(i=0; i<tim; i++){
		PORTG |= (1<<PG4);
		myDelay_us(time);
		PORTG &= ~(1<<PG4);
		myDelay_us(time);
	}
	PORTG |= (1<<PG4);
}


// LCD 출력 중 * 와 _ 입력했을 때의 따른 index값 찾는 함수
int find_last_index(unsigned char find_data) {
	int i = 0;
	for (i=0; i<sizeof(password_secret)/sizeof(unsigned char); i++) { //베열을 배열의 크기로 나눠서 원소의 개수를 구한다. 
		if (password_secret[i] == find_data) { // 입력한 모양에 따라 password_secret배열 원소 값에 넣어준다.
			return i; // 그 인덱스 값을 반환한다. 
		}
	}
	return i;
}

// 키패드에 대한 구조 이론 
/*
전체적 키패드 구조 
 1 2 3 m1
 4 5 6 m2
 7 8 9 m3
 * 0 # n4

C: 0 1 2 3 (입력)
R: 4 (출력)
   5
   6
   7

row : 4~7
col : 0~3

키패드 구조를 숫자로 변형해 놓은 모습 (음계_NUM 정의 확인)
1 2 3 4
5 6 7 8
9 10 11 12
13 14 15 16
*/

// 키패드 입력 함수
unsigned int KeyPad_input(){
	unsigned char auto_row = 0; // 0000 xxxx
	unsigned char current_code = 0;
	unsigned char col = 0;
	
	current_code = 0;
	for(auto_row = 0; auto_row<4; auto_row ++){
		Set_Row(auto_row);
		col = PINC & 0x0F; // 0000 1111
		
		switch(col){
			case 0x01: current_code = auto_row * 4 + 1; break; // 1번 col의 스위치가 눌린 경우 (PC0의미)  0 1 2 3 4 5 6 7
			case 0x02: current_code = auto_row * 4 + 2; break; // 2번 col의 스위치가 눌린 경우 (PC1의미)
			case 0x04: current_code = auto_row * 4 + 3; break; // 3번 col의 스위치가 눌린 경우 (PC2의미)
			case 0x08: current_code = auto_row * 4 + 4; break; // 4번 col의 스위치가 눌린 경우 (PC3의미)
		}
	}
	return current_code;
}

//키패드 입력 후 업뎃하는 함수
unsigned int update_keypad(unsigned int keypad_code) {
	unsigned char current_code = KeyPad_input();
	
	if (current_code == 0) { // 입력 안됐을때
		return 0;
	}
	
	// 입력이 된 상태
	if (keypad_code == 0) { // 입력은 됐는데, 사전에 키패드가 입력되어있지 않다.
		return current_code;
	}
	
	return keypad_code;
}

// 키패드 누를 때 소리 나오게 하는 함수 
void KeyPad_sound(unsigned int keypad_code){
	if(keypad_code == 0) {
		return;
	}
	
	// 키패드 동작에 따른 음계 출력
	if(keypad_code == DO_NUM){ SSound(DO, HALF_SEC); return;} 
	if(keypad_code == RE_NUM){ SSound(RE, HALF_SEC); return;}
	if(keypad_code == MI_NUM){ SSound(MI, HALF_SEC); return;}
	if(keypad_code == FA_NUM){ SSound(FA, HALF_SEC); return;}
	if(keypad_code == SOL_NUM){ SSound(SOL, HALF_SEC); return;}
	if(keypad_code == LA_NUM){ SSound(LA, HALF_SEC); return;}
	if(keypad_code == SI_NUM){ SSound(SI, HALF_SEC); return;}
	if(keypad_code == DO5_NUM){ SSound(DO5, HALF_SEC); return;}
	if(keypad_code == RE5_NUM){ SSound(RE5, HALF_SEC); return;}
	// 나머지
	SSound(MI5, HALF_SEC);
}

//버저 소리 3개 불러오기 함수
void buzz_three(int first, int second, int third, int time) {
	SSound(first, time);
	_delay_ms(500);  
	SSound(second, time);
	_delay_ms(500);
	SSound(third, time);
	_delay_ms(500);
}

//버저 소리 4개 불러오기 함수
void buzz_four(int first, int second, int third, int four, int time) {
	SSound(first, time);
	_delay_ms(500);
	buzz_three(second, third, four, time);
}

// 2. 금고 초기화 - 초기동작
void Safe_Init(){
	//4-솔 음 1회 100ms 출력
	SSound(SOL, SAFE_SEC);
	// 서보모터 0도로 
	close_the_door();
	// FND LCD 출력X 
	reset_display();
}

//LCD와 FND 초기화
void reset_display(){
	//LCD 내용 초기화 
	LCD_Clear();
	//FND 초기화
	no_fnd_disp();
}

// 각 음계 숫자로 입력 받는 함수
char getNumber(unsigned int keypad_code) {
	switch(keypad_code) {
		case DO_NUM: return '1';
		case RE_NUM: return '2';
		case MI_NUM: return '3';
		case FA_NUM: return '4';
		case SOL_NUM: return '5';
		case LA_NUM: return '6';
		case SI_NUM: return '7';
		case DO5_NUM: return '8';
		case RE5_NUM: return '9';
		case ZERO: return '0';	
	}
	return ' ';
}

// * 누른 후 비번 누르는 초기화 화면
void setSecret(unsigned char init) {
	int i=0;
	for (i=0; i<14; i++) { // LCD 출력시 쓰임
		password_secret[i] = init; //setSeccret 매개변수는 password_star = *  또는 underbar = _
	}
}
 
 // 배열 사이즈 구하는 함수
int sizeArr(unsigned char* origin){
	int i = 0;
	for(; *origin; origin++, i++) {}
	return i;
}

// 비밀번호 확인 함수 (맞는지 틀린지)
int check_password(int last_index, unsigned char* origin) {
	int i =0;
	int size = sizeArr(origin); 
	
	if (last_index != size) { // 입력한 비번 담긴 배열의 마지막 인덱스 값이 비교하는 초기 비번 담긴 배열의 크기와 비교하는데 
		return 0; //다르면 0 반환
	}

	for (i = 0; i < size; i++) {
		if (password_secret[i]== star_password) { // 입력한 비번이 담긴 배열의 원소 값이 * 이면
			return 0; // 0반환
		}
		
		if (password_secret[i] != origin[i]) { // 입력한 비번이 담긴 배열과 비교하는 초기 비번 담긴 배열의 원소 값을
			return 0; // 하나하나 비교해서 다르면 0반환 
		}
	}
	return 1; // 하나하나 비교해서 같으면 1 반환
}

// 비밀번호 모드 아닐때 관리 함수
void init_manage(unsigned int keypad_code){ //카운트 파라미터 추가
	
	if(keypad_code == SHARP){
		//# 눌렀을 때 LCD표시 생기고 비밀번호모드시작
		setSecret(star_password); //LCD판에 입력될 * 를 위한 함수 선언
		LCD_work(password_data, password_secret); // 비번 입력하면 LCD에 입력한 수 출력하는 함수
		
		current_mode = password_input_mode; // 현재 모드는 비밀번호 입력 모드 
	}
	
	
}

//도어 해제 후 동작 함수
void set_open(unsigned int *password_index) {
	LCD_work(door_open, empty); // LCD에 문열린 알림 확인
	*password_index=0; // 비밀번호 인덱스 값 선언
	_delay_ms(300);
	current_mode = door_open_mode; // 도어 해제 모드로 변경
}

// 비밀번호 모드 일때 관리하는 함수
void password_mode_manage(unsigned int keypad_code, unsigned int *password_index, int fnd_num){
	int equal_password = 0;
	char data;
	fnd_disp(fnd_num);//fnd에 숫자 나타내기 시작
	
	if (keypad_code == STAR) {
		//입력한 비번 값이 6~14자리가 맞는지 확인
		if(*password_index < 5 || *password_index > 13) {
			return;
		}
		
		//저장된 비번 값과 입력한 비번이 같은지 확인
		equal_password = check_password(*password_index, init_password);
		
		if(equal_password == 1){ //비번 정답
			//부저 울리기 4계음
			buzz_four(DO, MI, SOL, DO5, HALF_SEC * 10);
			// 서보모터 90도 변환후 도어 해제
			open_the_door();
			
			// Door Open 초기화
			set_open(password_index);
			// FND와 LCD 화면 초기화
			reset_display();
			count = 0;
			return;
		}
		
		// 비밀번호 틀렸을 때
		//부저 울리기 3계음
		count++; //비밀번호 틀린 횟수 체크
		
		if(count>= ERROR_COUNT){ // 오류 횟수 체크
			current_mode = dangerous_mode; // 비밀번호 입력 오류 3번 이상이면 위험모드 진입
			//LCD 위험모드 출력
			LCD_work(warning , theft);
			return;
		}
		// 틀렸을 때 라-4 3번 
		buzz_three(LA, LA, LA, HALF_SEC * 10);
		//LCD 표시
		LCD_work(password_data, password_error);
		_delay_us(ERR_SEC);
		// 입력값 = 비밀번호 입력 모드 진입 시 초기화
		*password_index=0;
		// 초기화면으로 되돌아 간다.
		reset_display(); // FND와 LCD 초기화
		current_mode = init_mode; // 초기 모드로 변경
		return;
	}
	
	if(*password_index >= 14) { //비밀번호 인덱스 값이 14이상이면(주어진 *과 _의 개수를 넘어선다면)
		return; // 반환
	}
		
	data = getNumber(keypad_code); // 키패드코드로 입력받은 수를 데이터로 입력
	if (data != ' ') { // 숫자이면
		//LCD 업뎃
		password_secret[*password_index] = data; // 비밀번호 인덱스를 password_secret배열의 한 원소당 데이터 하나 넣기
		(*password_index)++; // 비밀번호 인덱스 넣을 때마다 1씩 증가( 한 원소당 데이터 하나씩 넣으려고)
		LCD_work(password_data, password_secret); // 위 동작 LCD로 출력
	}
	
}

//비번 변경 후 저장하는 함수
void save() {
	int i = 0;
	int init_size = sizeof(init_password)/sizeof(unsigned char); //초기 비밀번호 담은 배열 크기 변수
	int secret_size = sizeof(password_secret)/sizeof(unsigned char); //비밀번호 담은 배열 크기 변수
	for (i=0; i<secret_size; i++) {
		init_password[i] = password_secret[i];
	}
	for (i = secret_size; i < init_size; i++) { 
		init_password[i] = 0;
	}
}

// FND 디스플레이하기 
void fnd_disp(int data)
{
     PORTE = Port_fnd[0]; PORTF = num_data[data/1000%10]; _delay_ms(1); PORTF=0xff; //천의자리
     PORTE = Port_fnd[1]; PORTF = num_data[data/100%10];  _delay_ms(1); PORTF=0xff; //백의자리
     PORTE = Port_fnd[2]; PORTF = num_data[data/10%10];   _delay_ms(1); PORTF=0xff; //십의자리
     PORTE = Port_fnd[3]; PORTF = num_data[data/1%10];    _delay_ms(1); PORTF=0xff; //일의자리

}

// 음계 숫자로 변경
int parse_number(unsigned char keyCode) {
	switch(keyCode) {
		case DO_NUM: return 1;
		case RE_NUM: return 2;
		case MI_NUM: return 3;
		case FA_NUM: return 4;
		case SOL_NUM: return 5;
		case LA_NUM: return 6;
		case SI_NUM: return 7;
		case DO5_NUM: return 8;
		case RE5_NUM: return 9;
		case ZERO: return 0;
		default:
			return -1;
	}
}

//fnd 자릿수 계산을 위한 함수
int make_fnd_num(int fnd_num, int number) {
	return fnd_num % 1000 * 10 + number;
}

//모터 잠금
void close_the_door() {
	OCR1A = 22117;  //0도
}

//모터 해제
void open_the_door(){
	OCR1A= 29491;     //90도
}

//외부 인터럽트0번 모드 동작 함수
void interrupt_active(unsigned char keypad_code, int fnd_num) {
	char data = getNumber(keypad_code); // 키패드로 숫자 입력받고 데이터 변수에 저장
	int last_index = find_last_index(underbar); // LCD애 출력되는_의 개수 만큼의 값을 last_index에 저장 

	fnd_disp(fnd_num); //FND에 키패드로 입력하는 숫자 출력
	
	if (data != ' ') { // 숫	자이면
		//LCD 업뎃
		password_secret[last_index] = data;
		last_index++;
		LCD_work(password_data, password_secret); // LCD 판에 비밀번호 
	}
}

//마스터 모드 확인 함수
void master_check(unsigned char keypad_code, int fnd_num){
	
	int equal_password = 0; //같은지 확인하는 변수
	char data = getNumber(keypad_code); //입력한 키패드 값을 데이터 변수에 저장
	int master_index = find_last_index(star_password); // 마스터 모드일때 이니까 그때 LCD에 나타낸*의 마지막 인덱스 값을 변수에 저장
	
	fnd_disp(fnd_num);//fnd에 숫자 나타내기
	
	
	if (keypad_code != STAR) { //*을 누르지 않는다면
		if (data == ' ') { // 숫	자가 아니면
			return; //반환
		}
		//LCD 업뎃
		password_secret[master_index] = data; // 입력한데이터를 비번 배열에 하나씩 저장
		master_index++; // 인덱스 증가
		LCD_work(master, password_secret); //마스터모드 일때 LCD에 입력하는 숫자 보여지게 출력
		
		return;
	}
	
	if (master_index == 0) { // 값이 채워지지 않으면
		return; //반환
	}
	
	//저장된 비번 값과 입력한 비번이 같은지 확인
	equal_password = check_password(master_index, master_key);
	
	is_dual=0; // 같은지 확인 했으니까 *이랑 # 누른 상태 OFF
	is_safe_mode = 0; //마스터모드도 OFF
	
	if(equal_password == 1){ //입력한 마스터 번호랑 저장된 마스터번호(학번)과 같으면 
		current_mode = init_mode; //모드는 다시 처음 초기 모드로
		reset_display(); //LCD와 FND 꺼지게 
		return;
	}
	
	LCD_work(warning , theft); //입력한 마스터 번호랑 저장된 마스터번호(학번)과 틀리면
	no_fnd_disp(); //다시 위험모드로 돌아가서 입력할 수 있게
}


//진동 감지 모드 함수
void vibration_check(){
	if (current_mode == dangerous_mode) {
		return;
	}
	
	// 0xxx xxxx PORTB & 80
	if(!(PINE & 0x01)){ //진동 센서 감지 시에 
		buzz_four(MI, DO, MI, DO ,HALF_SEC * 10); // 사이렌 경고음
		LCD_work(warning ,stealing);
		current_mode = dangerous_mode;
	}
	return;

}


// 외부 0번 인터럽트
ISR(INT0_vect){ //0번핀 동작
	int last_index = 0;
	
	if(current_mode != interrupt_mode){
		setSecret(underbar); //_로 LCD 출력할 문자 맞춰주기 
		LCD_work( password_set , password_secret);
		current_mode = interrupt_mode; // 현재모드는 인터럽트 모드로 
	}
	else{  
		last_index = find_last_index(underbar);
		if(last_index< 5 || last_index > 13) { // 새로 기입한 비번의 길이 4~16인지 확인 
			return;
		}
		//부저 울리기 3계음
		buzz_three(DO, RE ,MI, HALF_SEC * 10); // 기입 후 외부인터럽트 누르면 부저 재생
		LCD_work( password_set , success); //저장 완료 시 LCD 판에 출력
		_delay_ms(30);
		
		save();//바꾼 비번 저장
		reset_display();// 비번 저장 후 초기화 해주는거
		current_mode = init_mode; //저장 후 초기 상태로

	}
	
}


//타이머 인터럽트
ISR(TIMER2_COMP_vect){
	
	vibration_check();
	
	if (current_mode== dangerous_mode) { //위험모드이면
		if ((PINC & 0x0f) == 0x05) { // * 과 # 동시 누름
			dual_count++; // 같이 누른 상태의 초를 의미하는 변수
			if (dual_count == 60000) { // 3초면
				is_dual = 1; //같이누른 상태다 1
			}
		}
		else { //아니면
			dual_count = 0; //같이 누른 상태 아니다 0
		}
	}
	
	if (open_star == 0) { //열려있을 때 *누르지 않은 상태
		timer_count = 0; // 초 변수 변화 없음.
	}
	else { //열려있을 때 *누른 상태
		timer_count++; // 초 변수 변화
		
		if (timer_count == 60000) { //3초 되면
			buzz_four(SOL, FA, MI, RE, HALF_SEC * 10);// 도어 잠길때 부저 재생
			
			// 도어 잠금, 모터 0도 (도어 잠금)
			close_the_door();
			
			open_star = 0;
			current_mode = init_mode; // 초기모드로 초기화
			reset_display(); //FND LCD 초기화
		}
	}
}

int main(void)
{	
	int fnd_num = 0;
	int each_fnd = 0;
	unsigned char keypad_code = 0; //숫자 or 문자 인지 판별할때  
	
	
	int password_index = 0; // 비밀번호 받는 배열의 인덱스 값
		
	// 1번 : 키패드 인터페이스
	port_init();
	init_interface();
	
	// 2번: 금고 초기화
	Safe_Init();

    while (1) 
    {	 
		

		// 1번 : 키패드 인터페이스
		keypad_code = update_keypad(keypad_code);
		KeyPad_sound(keypad_code);
		
		// fnd 입력을 위해 fnd 숫자로 바꾸기
		each_fnd = parse_number(keypad_code); //입력한 키패드에 맞는 음을 숫자로 표현

		if (each_fnd != -1) { // 입력하는 비번의 숫자들을 의미
			fnd_num = make_fnd_num(fnd_num, each_fnd);	//fnd에 입력하는 수 보일 수 있게 4자리수로 만드는 함수 동작
		} 
	
		
		// 3번 : 금고 비밀번호 일치 후 개방
		if(current_mode == init_mode){ //초기모드면
			//PORTB &= 0xfe;
			init_manage(keypad_code); // 초기 비번 입력 함수 동작
		}
		else if(current_mode == password_input_mode){ //비번 입력모드면
			//PORTB &= 0xfd;
			password_mode_manage(keypad_code, &password_index, fnd_num); //비밀번호 관리 함수 동작
		}
		// 4번 : * 3초 누르고 금고 잠그기
		else if (current_mode == door_open_mode) { // 도어 잠겨있는 상태면 
			
			if (keypad_code == STAR) {
				open_star = 1;
			}
			else {
				open_star = 0;
			}
		}
		// 5번 인터럽트 동작 
		else if(current_mode == interrupt_mode){ // 인터럽트 상태이면 
			interrupt_active(keypad_code, fnd_num); //인터럽트 모드일 때 동작 함수
		}
		// 6번 보안대책
		else if(current_mode == dangerous_mode){ // 위험모드 이면
			
			if (is_safe_mode == 1) { //마스터 모드이면
				//PORTB = 0x0b;	// 0000 1011	
				master_check(keypad_code,fnd_num); // 마스터모드일때 동작체크 함수
			}
			else {
				
				buzz_four(MI, DO, MI, DO ,HALF_SEC * 10); // 사이렌 경고음
				if (is_dual) {
					//PORTB = 0x03; // 1100 0000
					// lcd 표시
					setSecret(star_password); // *로 자릿수 채우기
					LCD_work(master, password_secret ); //마스터모드 진입 LCD 출력

					is_safe_mode = 1; //마스터모드 ON
					fnd_num = 0;
				}
				else {
					//PORTB = 0x0c; // 0011 0000
					is_safe_mode= 0; // 마스터모드 OFF
				}
			}
			
		}
		else{ //위험모드가 아닐때 
		}
		_delay_us(SAFE_SEC);
	}
	return 0;
}

