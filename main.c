#include <REG51.H>


// cac trang thai cong tac
#define DIPSWITCH_000 	0		
#define DIPSWITCH_001	1		
#define DIPSWITCH_010	2
#define DIPSWITCH_011	3
#define DIPSWITCH_100	4
#define DIPSWITCH_101	5
#define DIPSWITCH_110	6
#define DIPSWITCH_111	7


//cac bien toan cuc
unsigned int g_dipswitch_state = 0;
unsigned int g_arr_RPM [3];
unsigned int g_falling_edge_counter = 0;
unsigned int g_avg_RPM = 0;

char g_seg_code[]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90}; // 1, 2, 3... 9

// khai bao cac chan
sbit dipswitch_a = P1^0;		//chan P1.0 la dipswitch_a
sbit dipswitch_b = P1^1;		//chan P1.1 la dipswitch_b
sbit dipswitch_c = P1^2;		//chan P1.2 la dipswitch_c

sbit pwm_output = P2^0;			//chan P2.0 la pwm output
sbit sw_s1_7seg = P2^3;			//chan P2.3 la s1 of 7 segment
sbit sw_s2_7seg = P2^2;			//chan P2.2 la s2 of 7 segment
sbit sw_s3_7seg = P2^1;			//chan P2.1 la s3 of 7 segment

//cac ham
void pwm_output_motor(int);
void check_dipswitch_state();
void motor_RPM();
void edge_counter() ;
void delay(int);
void feedback_control (int);
void display_RPM();

// ngat
// init ngat
void interrupt_init (){
	// dung ngat ngoai khi gap suon am
	IT0 = 1;   // Configure Interrupt 0 for falling edge on /INT0 (P3.2)
	EX0 = 1;   // Enable EX0 Interrupt
	EA = 1;    // Enable Global Interrupt Flag to Start Interrupt
	
	/*Use timer 1 to generate an interupt at every 50ms, and use timer 0 to  create delay*/
	TMOD = 0x11; //t1 used to create interupt of (50ms), and t0 to create delay (0001 0001)
	TH1 = 0x4B;  //Counting from 0x4BFF to 0xFFFF to takes 50ms. After 50ms Interrupt routine is called.
	TL1 = 0xFF; 
	TR1 =1; //turn on timer 1 
	ET1 = 1; //enable timer 1 interupt
}


// dem so suon am 
void falling_edge_interrupt () interrupt 0{
	g_falling_edge_counter++; 
}


// ngat 50ms va tinh rpm
// ngat moi 50ms va tinh rpm trung binh
void interrupt_50ms() interrupt 3{	
	int RPM; 
	
	//RPM = (X/0.05s * 60s/min)/24 = X * 50 
	// X la so suon am
	RPM = (g_falling_edge_counter)*50; 	
		
	// cho cac RPM vao list de tinh trung binh
	g_avg_RPM = (g_arr_RPM [0] + g_arr_RPM [1] + g_arr_RPM[2])/3;
		
	// giu lai 3 gia tri RPM moi nhat de tinh RPM trung binh
	g_arr_RPM [0] = g_arr_RPM [1];
	g_arr_RPM [1] = g_arr_RPM [2];
	g_arr_RPM [2] = RPM;
	
		
	g_falling_edge_counter = 0;	// reset couter cua suon am
	TH1 = 0x4B;  // reset timer cho 50ms ngat tiep thao
	TL1 = 0xFF;
}



void main(void)
 { 
   interrupt_init ();			//init ngat
   dipswitch_a = 1;			//set cac cong 1.0, 1.1, 1.2 de doc dipswitch value
   dipswitch_b = 1;
   dipswitch_c = 1;
   
  
   pwm_output = 0;			// set cong 2.0 la output cho pwm
   sw_s1_7seg = 0;
   sw_s2_7seg	= 0;
   sw_s3_7seg = 0;
   P0 = 0x00;				// set cong 0 la output
    

	// Hard code cac gia tri
    while (1) {
		 switch (g_dipswitch_state){ // siwtch cac gia tri cua dipswich
				case DIPSWITCH_000:
					 feedback_control(0); //0% 
					 break;
				case DIPSWITCH_001:
					 feedback_control(240); //10% 
					 break;
				case DIPSWITCH_010:
					 feedback_control(720); //30% 
					 break;
				case DIPSWITCH_011:
					 feedback_control(1200); //50% 
					 break;
				case DIPSWITCH_100:
					 feedback_control(1440); //60% 
					 break;
				case DIPSWITCH_101:
					 feedback_control(1680); //70% 
					 break;
				case DIPSWITCH_110:
					 feedback_control(1920); //80%
					 break;
				case DIPSWITCH_111:
					 feedback_control(2400); //100% 
					 break;
				default:
					 feedback_control(0);
				}
	   }
}
 



// su dung RPM hien tai va RPM mong muon de tinh duoc error
// sau do su dung error de tinh lai pwm
// hien thi rpm hien tai
void feedback_control(int desired_RPM){
	int output = (((desired_RPM - g_avg_RPM))*5); //output = error*proportional_gain
	//update motor
	pwm_output_motor (output);
	display_RPM ();
}

// dieu chinh rpm output dua tren loi tinh duoc boi lam tren
// hieu chinh rpm khi >100 hoac <0
void pwm_output_motor(int dutycycle){
	
		//check if dutycycle is 0 or less, output 0 pwm
   if (dutycycle<=0){
		 pwm_output = 0;
      }
   //check if dutycycle is 100 or more, output 100 pwm 
   else if (dutycycle>=100){
			pwm_output = 1;
      } 
   else{
      //set pwm_output as high
      pwm_output = 1;
      delay (dutycycle);
      //set pwm output as low
      pwm_output = 0;
      delay(100-dutycycle);
      }
	 	//check dip switch state
  check_dipswitch_state();
}


// check trang thai cua tung switch trong dipswitch
void check_dipswitch_state(){
   // 000 (a= high, b =high , c= high)
   if (dipswitch_a == 1 && dipswitch_b == 1 && dipswitch_c == 1){
      g_dipswitch_state = DIPSWITCH_000;
      }  
   //001 (a= high, b =high , c= low)
   else if (dipswitch_a == 1 && dipswitch_b == 1 && dipswitch_c == 0){
      g_dipswitch_state = DIPSWITCH_001;
      }  
   //010 (a= high, b =low , c= high)
   else if (dipswitch_a == 1 && dipswitch_b == 0 && dipswitch_c == 1){
      g_dipswitch_state = DIPSWITCH_010;
      }  
   //011(a= high, b=low , c= low)
   else if (dipswitch_a == 1 && dipswitch_b == 0 && dipswitch_c == 0){
      g_dipswitch_state = DIPSWITCH_011;
      }  
   //100 (a= low, b =high , c= high)
   else if (dipswitch_a == 0 && dipswitch_b == 1 && dipswitch_c == 1){
      g_dipswitch_state = DIPSWITCH_100;
      }  
   //101 (a= low, b =high , c= low)
   else if (dipswitch_a == 0 && dipswitch_b == 1 && dipswitch_c == 0){
      g_dipswitch_state = DIPSWITCH_101;
      }  
   //110 (a= low, b =low , c= high)
   else if (dipswitch_a == 0 && dipswitch_b == 0 && dipswitch_c == 1){
      g_dipswitch_state = DIPSWITCH_110;
      }  
   //111(a= low, b =low , c=low)
  else if (dipswitch_a == 0 && dipswitch_b == 0 && dipswitch_c == 0){
      g_dipswitch_state = DIPSWITCH_111;
      }  
}


// hien thi RPM
// chi hien thi 3 so dau cua RPM
// vd 720 RPM se hien thi 720
// 1200 se hien thi 120
void display_RPM (){
	int first_digit; 
	int second_digit; 
	int third_digit; 
	
	int temp; 
	int RPM_temp; 
	
	
	if (g_avg_RPM<=0){ 
		// RPM <= 0	
		first_digit = g_seg_code[0];
		second_digit = g_seg_code[0];
		third_digit = g_seg_code[0];
	}
	else if (g_avg_RPM>0 && g_avg_RPM<10){ 
		//0<RPM<10
		first_digit = g_seg_code[0];
		second_digit = g_seg_code[0];
		third_digit = g_seg_code[g_avg_RPM];
	}

	else if (g_avg_RPM>=10 && g_avg_RPM<100){ //10<RPM<100
		//10<=RPM<100
		first_digit = g_seg_code[0];
		second_digit = g_seg_code[g_avg_RPM/10]; 
		third_digit = g_seg_code[g_avg_RPM%10]; 
	}
	else if (g_avg_RPM>=100 && g_avg_RPM<1000){ 
		//100<RPM<1000
		temp = g_avg_RPM/10;
		
		first_digit = g_seg_code[temp/10];
		second_digit = g_seg_code[temp%10];
		third_digit = g_seg_code[g_avg_RPM%10];
	}
	else if (g_avg_RPM>=1000 && g_avg_RPM<10000){
		// 1000<=RPM<10000
		temp = g_avg_RPM/10;
		RPM_temp = temp; 
		temp = RPM_temp/10;
		
		first_digit = g_seg_code[temp/10];
		second_digit = g_seg_code[temp%10];
		third_digit = g_seg_code[RPM_temp%10];
	}
	else {
		// error
		first_digit = 0x06;
		second_digit = 0x06;
		third_digit = 0x06;
	}
		sw_s1_7seg = 1;
		sw_s2_7seg = 0;
		sw_s3_7seg = 0;

		P0 = first_digit;
		delay (20);

		sw_s1_7seg = 0;
		sw_s2_7seg = 1;
		sw_s3_7seg = 0;

		P0 = second_digit;
		delay (20);

		sw_s1_7seg = 0;
		sw_s2_7seg = 0;
		sw_s3_7seg = 1;

		P0 = third_digit;
		delay (20);
}


// ham delay 100us
void delay(int num_of_100us_delay){
   int i;
   for (i=0; i<num_of_100us_delay; i++){
	 /*one for loop generates a delay of 100u sec 
		 count from 0xFFA3 to 0xFFFF. Each count takes 1.085u sec, 
		 therefore counting from 0xFFA3 to 0xFFFF will result in 100u sec of delay */
		 TMOD = 0x11; //t1 used to create Interrupt of 50ms, and t0 to create delay of 100us (0001 0001)
		 TH0 = 0xFF;  //set MSB to 1111 1111 (0xFFA3= 1111 1111 1010 0011)
		 TL0 = 0xA3; //set LSB to 1010 0011
		 TR0 =1; //turn on timer zero to count 
		 while(TF0 == 0){
			 //while overflow flag is zero, do nothing 
		 } 
		 TF0= 0; //clear the overflow flag
		 TR0 = 0; //clear timer
	 }
}   
