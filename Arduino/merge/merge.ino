#include "LedControl.h"
#include <SimpleTimer.h>
#include <IRremote.h>
#include <SoftwareSerial.h>

#define minTime 1
#define dw digitalWrite
#define dr digitalRead
#define aw analogWrite
#define ar analogRead

#define RangeA 15
#define RangeB 25
#define RangeC 35
#define RangeD 45

const byte dec_digits[] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101, 0b01111100, 0b00000111, 0b01111111, 0b01100111 };
byte seg_wave[]={0b00000111,0b00001000,0b00001110,0b00000001};
byte wave_Medium[7] = {B00000000,B01100000,B11110000, B11111001,B11111111};
#define seg_data  4   //orange
#define seg_clk   5   //yellow
#define seg_lat   6   //blue
#define dht_PIN   9   //orange
#define dot_din   10  //orange
#define dot_cs    11  //yellow
#define dot_clk   12  //green
#define joy_bt    13  //green

#define wat_lev   A0  //green
#define temp_db   A1  //blue
#define joy_xp    A2  //blue
#define joy_yp    A3  //black

int mode,set_temp=35,set_time,set_servo;//main mode
int seg_num,seg_dot,seg_mode,seg_blk;//4*7segment
int dot_mode,dot_num,dot_trg;        //8*8dot
int temp,waterlevel;                 //temp,water senser
int ir_trg=0,ir_data=0;               //ir control
int bt_time=0,bt_trg=0;char bt_temp=0;//bluetooth
int joy_state=0;                      //joystic
SoftwareSerial btSerial(7,8);         //bluetooth
SimpleTimer timer(minTime);           // 1ms timer interupt
LedControl lc = LedControl(dot_din,dot_clk,dot_cs,1); //8*8 dotmatix
IRsend irsend;                        // IRremote

byte shiftbyte(byte b){             // dotmatix's byte shift
  byte temp=b&0b00000001;
  temp=temp<<7;
  b=b>>1|temp;
  return b;
}
double GetTemperature(int v)        //analog temp val -> double temp val
{
 double Temp;
 Temp = log(10000.0 / (1024.0 / v - 1)); 
 Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp))* Temp);
 Temp = Temp - 273.15; // Convert Kelvin to Celcius
// Temp = (Temp * 9.0) / 5.0 + 32.0; // Convert Celcius to Fahrenheit
 return Temp;
}

void setup() {
  Serial.begin(9600);         //시리얼 모니터 on
  btSerial.begin(9600);       //블루투스 on

  // 8*8 dot init
  lc.shutdown(0,false);       // 첫 번째 드라이버의 절전모드를 해제
  lc.setIntensity(0,4);       // 밝기를 중간으로 설정
  lc.clearDisplay(0);         // 디스플레이 초기화
  lc.setRow(0,0,0b0);
  lc.setRow(0,1,0b0);
  lc.setRow(0,2,0b0);
  lc.setRow(0,3,0b0);
  lc.setRow(0,4,0b0);
  lc.setRow(0,5,0b0);
  lc.setRow(0,6,0b0);
  lc.setRow(0,7,0b0);

  // 4*7 segment init
  pinMode(seg_data,OUTPUT);
  pinMode(seg_lat,OUTPUT);
  pinMode(seg_clk,OUTPUT);

  pinMode(wat_lev,INPUT);        //waterlevel
  pinMode(temp_db,INPUT);        //temp senser
  pinMode(joy_xp,INPUT);
  pinMode(joy_yp,INPUT);
  pinMode(joy_bt,INPUT);
}

void loop() {
  if(timer.isReady()){
    timer.reset();
    temp_ms();
    main_seq_ms();
    seg_ms();
    dot_ms();
    ir_ms();
    bt_ms();
    joy_ms();
  }
}


void temp_ms(){
  #define AVER 10
  static int temp_seqcnt=0,temp_t[AVER],temp_seq=0;
  long sum=0;
  temp_seqcnt++;
  if(temp_seqcnt>=100){
    temp_seqcnt=0;
    waterlevel=analogRead(wat_lev);    //water level senser
 //   Serial.println(waterlevel);
    if(waterlevel<350){
      waterlevel=0;
    } else if(waterlevel>=680&&waterlevel<=820){
      waterlevel=100;
    } else {
      waterlevel=map(waterlevel,300,680,1,100);
    }
    
    temp_t[temp_seq++]=int(GetTemperature(ar(temp_db))*100);   //temp senser
    if(temp_seq>=AVER)temp_seq=0;
    for(int i=0;i<AVER;i++)sum+=temp_t[i];
    temp=int(sum/AVER);
    seg_dot=3;

    if(seg_mode==0)seg_num=temp;
    if(seg_mode==1)seg_num=set_temp;
    if(seg_mode==1&&mode==2)seg_num=set_time;

  //  irsend.sendSony(ir_data,12);//======================================
  }
}

void main_seq_ms(){
  static int wait_cnt,blk_cnt,cht_trg,joy_dcnt;
  switch(mode){
    case 0: // 대기상태
      dot_mode=0;
      if(seg_mode==0)seg_mode=2;
      if(bt_temp!=0){   //bluetooth data reseive
        mode=2;
        wait_cnt=1000;
        bt_temp=0;
      }
      if(joy_state!=0){             //joystic move
        seg_mode=1;
        mode=1;
        set_temp=35;
        blk_cnt=500;
        
      }
      break;
    case 1: // 조이스틱 온도설정
      seg_mode=1;
      dot_mode=3;
      blk_cnt--;
      if(blk_cnt<=0){
        blk_cnt=500;
        seg_blk=~seg_blk;
      }
      if(joy_state!=0){
        blk_cnt=500; seg_blk=0;
        joy_dcnt++;
        if(joy_dcnt>=250){
          joy_dcnt=0;
          if(joy_state==1){
            if(set_temp<45)set_temp+=10;
          }else if(joy_state==2){
            if(set_temp>15)set_temp-=10;
          }
        }
        
      }
      if(dr(joy_bt)==LOW){  //mode 3으로 이동
        cht_trg=1;
        mode=3;
        seg_mode=0;
        set_servo=0;
        seg_blk=0;
      }
      break;
    case 2: // 블루투스 카운트다운
      wait_cnt--;
      seg_mode=1;
      if(wait_cnt<=0){
        wait_cnt=1000;
        set_time--;
      }
      if(set_time==0){
        mode=3;
        seg_mode=0;
        dot_mode=1;
      }
      break;
    case 3: // 물나오고 있는중
      seg_mode=0;
      dot_mode=1;
      if(waterlevel<100&&set_servo==0){
        switch(set_temp){
          case RangeA:
            ir_trg=1;Serial.println("set1");
            ir_data=0b0011;
            set_servo=ir_data;
            break;
          case RangeB:
            ir_trg=1;Serial.println("set2");
            ir_data=0b0110;
            set_servo=ir_data;
            break;
          case RangeC:
            ir_trg=1;Serial.println("set3");
            ir_data=0b1001;
            set_servo=ir_data;
            break;
          case RangeD:
            ir_trg=1;Serial.println("set4");
            ir_data=0b1100;
            set_servo=ir_data;
            break;
            default: Serial.println("other");
        }
      }
      if(joy_state!=0){
        mode=1;
      }
      if(waterlevel>=100&&set_servo!=0){Serial.println("wat");
        ir_trg=1;
        ir_data=0b0000;
        set_servo=ir_data;
      }
      if(cht_trg==1&&dr(joy_bt)==HIGH){Serial.println("chatout");
        cht_trg=0;
      }
      if(cht_trg==0&&dr(joy_bt)==LOW){Serial.println("chatin");
        cht_trg=1;
        if(set_servo!=0b10000){
          ir_trg=1;Serial.println("ss");
          ir_data=0b0000;
          set_servo=0b10000;
        }else{
          set_servo=0;
        }
      }
      break;
  }
}

void seg_ms() {
#define Fseg 3
  static int seg_seq=0,seg_seqcnt=0;
  static int seg_dseq=0,seg_dcnt=0; //seg_wave use
  byte dt;
  int n1=0,seq_c=(1<<seg_seq);
  
  seg_seqcnt++;
  if(seg_seqcnt>=Fseg){
    dw(seg_lat,HIGH);
    shiftOut(seg_data,seg_clk,MSBFIRST,seq_c);
    switch(seg_seq){
     case 0: n1=seg_num/1000%10; break;
     case 1: n1=seg_num/100%10; break;
     case 2: n1=seg_num/10%10; break;
     case 3: n1=seg_num%10; break;
    }
   switch(seg_mode){
    case 0: 
      dt=dec_digits[n1]; 
      if(seg_dot-1==3-seg_seq)dt|=0x80; 
      break;
    case 1: 
      if(seg_seq<=1)dt=0;
      else 
        dt=dec_digits[n1]; 
      break;
    case 2:
      dt=seg_wave[seg_seq]; 
      seg_dcnt++;
      if(seg_dcnt>=(130/Fseg)){
        seg_dcnt=0;
        seg_dseq++; if(seg_dseq>3)seg_dseq=0;
        byte temp=seg_wave[3];
        seg_wave[3]=seg_wave[2];
        seg_wave[2]=seg_wave[1];
        seg_wave[1]=seg_wave[0];
        seg_wave[0]=temp;
      }
      break;
   }
   if(seg_blk&(1<<(3-seg_seq)))dt=0;   //seg off 0bxxxx
   shiftOut(seg_data,seg_clk,MSBFIRST,~dt);
   
    seg_seqcnt=0;
    seg_seq=seg_seq+1>=4?0:seg_seq+1;
    dw(seg_lat,LOW);
  }
}
void dot_ms(){
  static int dot_seq=0,dot_seqcnt=0,dot_t;
  dot_seqcnt++;
  if(dot_mode==1){
    if(dot_seqcnt>=300){
      dot_seqcnt=0;
      if(waterlevel<30){
        //low
        lc.setRow(0,0,wave_Medium[0]);
        lc.setRow(0,1,wave_Medium[0]);
        lc.setRow(0,2,wave_Medium[0]);
        lc.setRow(0,3,wave_Medium[2]);
        lc.setRow(0,4,wave_Medium[4]);
        lc.setRow(0,5,wave_Medium[4]);
        lc.setRow(0,6,wave_Medium[4]);
        lc.setRow(0,7,wave_Medium[4]);
      }else if(waterlevel<60){
        //mid
        lc.setRow(0,0,wave_Medium[0]);
        lc.setRow(0,1,wave_Medium[0]);
        lc.setRow(0,2,wave_Medium[1]);
        lc.setRow(0,3,wave_Medium[2]);
        lc.setRow(0,4,wave_Medium[3]);
        lc.setRow(0,5,wave_Medium[4]);
        lc.setRow(0,6,wave_Medium[4]);
        lc.setRow(0,7,wave_Medium[4]);
      }else {
        //high
        lc.setRow(0,0,wave_Medium[0]);
        lc.setRow(0,1,wave_Medium[1]);
        lc.setRow(0,2,wave_Medium[1]);
        lc.setRow(0,3,wave_Medium[2]);
        lc.setRow(0,4,wave_Medium[2]);
        lc.setRow(0,5,wave_Medium[3]);
        lc.setRow(0,6,wave_Medium[3]);
        lc.setRow(0,7,wave_Medium[4]);
      }
      for(int i=0;i<7;i++){
          wave_Medium[i]=shiftbyte(wave_Medium[i]);
        }
    }
  } else if(dot_mode==2){
    if(dot_trg==1){
      dot_trg=0;
      dot_seq=64;
      dot_seqcnt=dot_t=set_time*1000/64;
      dot_seqcnt=0;
    }
    dot_seqcnt++;
    if(dot_seqcnt>=dot_t){
      dot_seqcnt=0;
      dot_seq--;
      
      for(int i=0;i<8;i++){
        byte buf=0;
        for(int j=min(dot_seq-((7-i)*8),7);j>=0;j--)buf|=1<<j;
        lc.setRow(0,i,buf);
      }
    }
  } else if(dot_mode==3){
    
  }
}
void ir_ms(){
  static int cnt;
  if(ir_trg==1){
    ir_trg=2;
    irsend.sendSony(ir_data,12);
    Serial.println(ir_data);              //debug------------------------------
  }
  if(ir_trg>=2){
    cnt++;
    if(cnt>=60){
      cnt=0;
      ir_trg++;
      irsend.sendSony(ir_data,12);
      Serial.println(ir_data);              //debug----------------------------
      if(ir_trg>=3){
        ir_trg=0;
      }
    }
  }
}
void bt_ms(){
  static int cnt=0;
  cnt++;
  if(cnt>=1000){
    cnt=0;
    //블루투스에 35도|30% 또는 100 (알람) 문자열 전송
    if(waterlevel>=90)bt_trg=1;
    if(bt_trg==0){  // 35도|30%
      char buf[20]={}; sprintf(buf,"%d℃    %2d%%",int(temp/100),waterlevel);// Arduino to Bluetooth data 
      btSerial.write(buf);
  //    Serial.println(buf);                                  //debug
    }else{          // 100
      bt_trg=0;
      btSerial.write("100");
  //    Serial.println("100");                              //debug
    }
  }
  if(btSerial.available()){
    bt_temp = btSerial.read();
    
    switch(bt_temp) {
      //차갑게
      case 'A':
        bt_time=btSerial.parseInt();
        if(btSerial.read()=='.')break;
        break;
      //시원
      case 'B':
        bt_time=btSerial.parseInt();
        if(btSerial.read()=='.')break;
        break;
      //따뜻
      case 'C':
        bt_time=btSerial.parseInt();
        if(btSerial.read()=='.')break;
        break;
      //뜨겁
      case 'D':
        bt_time=btSerial.parseInt();
        if(btSerial.read()=='.')break;
        break;
      }
      set_temp=15+(int(bt_temp)-'A')*10;
      set_time=bt_time;
      dot_trg=1;
   //   Serial.print(bt_temp); Serial.print("    "); Serial.println(bt_time); //debug
    }
}
void joy_ms(){
  static int cnt=0;
  int x,y;
  cnt++;
  if(cnt>=200){
    cnt=0;
    x=analogRead(joy_xp);
    y=analogRead(joy_yp);
    if(x>=0&&x<=200&&y>=300&&y<=700){              //joy up
      joy_state=1;
    }else if(x>=800&&x<=1023&&y>=300&&y<=700){    //joy down
      joy_state=2;
    }else if(x>=400&&x<=700&&y>=800&&y<=1023){    //joy left
      joy_state=3;
    }else if(x>=400&&x<=650&&y>=0&&y<=200){    //joy right
      joy_state=4;
    }else{                                        //middle
      joy_state=0;
    }
  //  char buf[20]; sprintf(buf,"%04d %04d %d %d\n",x,y,joy_state,dr(joy_bt)); Serial.print(buf); //debug
  }
}
