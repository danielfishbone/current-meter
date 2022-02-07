#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <CurrentTransformer.h>     // https://github.com/JChristensen/CurrentTransformer
#include <Streaming.h>              // http://arduiniana.org/libraries/streaming/
#include <Rotary.h>

#define  relay4 9
#define  relay3 10
#define  relay2 11
#define  relay1 12
#define  relay0 5
#define  buzzer 4 
#define  remAddress0  1
#define  multAddress0 2
#define  remAddress1  5
#define  multAddress1 6
#define  remAddress2  7
#define  multAddress2 8
#define  remAddress3  9
#define  multAddress3 10
#define  remAddress4  11
#define  multAddress4 12
#define  bk_address 3
#define  to_address 4
#define  OK 1
#define  s0 6
#define  s1 7
#define  s2 8

float current0;
float current1;
float current2;
float current3;
float current4;

uint32_t msTime;
uint32_t msBuzz;
uint32_t msWait0;
uint32_t msWait1;
uint32_t msWait2;
uint32_t msWait3;
uint32_t msWait4;

uint32_t msWaitout0;
uint32_t msWaitout1;
uint32_t msWaitout2;
uint32_t msWaitout3;
uint32_t msWaitout4;

bool useDefault=true;
bool busy=false;
bool saveFlag=false;
bool timeFlag=false;


const uint8_t ctChannel0(0);                // adc channel for ct-0
const float ctRatio(1000);                  // current transformer winding ratio
const float rBurden(220);                   // current transformer burden resistor value
const uint32_t tripTime =1000;

 uint32_t waitOut;
int en =A1;

String space="            ";
float preset0;
float preset1;
float preset2;
float preset3;
float preset4;
float percent =0.8;

bool  tr0=false;
bool  tr1=false;
bool  tr2=false;
bool  tr3=false;
bool  tr4=false;

volatile int setCurr;
volatile int t1=3;
volatile bool setFlag=false;
volatile bool yes = false;
volatile short page;

short x;

const int relays[5]={relay0,relay1,relay2,relay3,relay4};
float currents[5]={current0,current1,current2,current3,current4};
uint32_t msWait[5]={msWait0,msWait1,msWait2,msWait3,msWait4};
uint32_t msWaitout[5]={msWaitout0,msWaitout1,msWaitout2,msWaitout3,msWaitout4};
const short remAddress[5]={remAddress0,remAddress1,remAddress2,remAddress3,remAddress4};
const short multAddress[5]={multAddress0,multAddress1,multAddress2,multAddress3,multAddress4};
float preset[5]={preset0,preset1,preset2,preset3,preset4};
bool tripFlag[]={tr0,tr1,tr2,tr3,tr4};
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 20 chars and 4 line display
CT_Sensor ct0(ctChannel0, ctRatio, rBurden);
CT_Control ct;
Rotary r = Rotary(3, 2);
 
void setup() {
  // put your setup code here, to run once:
    pinMode(en,OUTPUT);
    pinMode(relay0,OUTPUT);
    pinMode(relay1,OUTPUT);
    pinMode(relay2,OUTPUT);
    pinMode(relay3,OUTPUT);
    pinMode(relay4,OUTPUT);
    pinMode(s0,OUTPUT);
    pinMode(s2,OUTPUT);
    pinMode(s1,OUTPUT);
    pinMode(A1,OUTPUT);
    pinMode(buzzer,OUTPUT);    
    pinMode(OK,INPUT_PULLUP);
      
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);
    sei();


  
    lcd.init(); 
    float vcc = ct.begin();
    lcd.backlight();
if(digitalRead(OK)==false){
    settings();
    }
    if(useDefault==true){
    x=EEPROM.read(bk_address);
    if(x==1)lcd.backlight();
    else if(x==0) {lcd.noBacklight();}
    t1=EEPROM.read(to_address);
    waitOut=t1*1000;
    }
    
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Current Limiter");
    delay(1000);
    for(int i=0;i<5;i++)
    {
    preset[i]=getCurrent(i);
    }
    lcd.clear();
    
    msTime = millis();
    msBuzz = msTime;
    for(int i=0;i<5;i++){
    msWait[i] = msTime;
    msWaitout[i] = msTime;
    
    }
    digitalWrite(relay0,HIGH);
    digitalWrite(relay1,HIGH);
    digitalWrite(relay2,HIGH);
    digitalWrite(relay3,HIGH);
    digitalWrite(relay4,HIGH);
    digitalWrite(A1,LOW);
}


void loop() {
  // put your main code here, to run repeatedly:
  readCurrent();
  if(millis()-msBuzz >1000){
    lcd.clear();
    msBuzz =millis();
  }
  lcd.setCursor(0, 0);
 //lcd << F("Preset(") << (page) << F(")")<< (preset[i]);
  lcd.print("Preset(A):  ");
  lcd.print(preset[page]);
  lcd.print("A");
  lcd.setCursor(0, 1);
  lcd << F("1-5 (") << (page+1) << F("):    ");
  //lcd.print("1-5 (A):    "); 
  lcd.print(currents[0],2);
  lcd.print("A"); 
  lcd.setCursor(2, 2);
  lcd.print(currents[1],2);
  lcd.print("A");
  lcd.setCursor(12,2 );
  lcd.print(currents[2],2);
  lcd.print("A");
    lcd.setCursor(2, 3);
  lcd.print(currents[3],2);
  lcd.print("A");
    lcd.setCursor(12, 3);
  lcd.print(currents[4],2);
  lcd.print("A");
  
  if(currents[0] > (preset[0]*percent) ||currents[1] > (preset[1]*percent)||currents[2] > (preset[2]*percent)||currents[3] > (preset[3]*percent)||currents[4] > (preset[4]*percent) )
  {
    if(millis()-msTime >= 500 && millis()-msTime < 1000)
        {
         beep(2,50);
         }
 if(millis()-msTime >= 1000)
        {
          digitalWrite(buzzer,LOW);
         msTime=millis();
          } 

    }
   for(int i=0;i<5;i++)
   {
    if (currents[i] >= preset[i])
    {
      if(millis()- msWaitout[i] >= tripTime)
      {
      digitalWrite(relays[i],LOW);  
      msWait[i]=millis();
      tripFlag[i] =true;
      }
      }

//if( millis()- msWaitout[i] >= tripTime && tripFlag[i] ==true)
//     {
//         digitalWrite(relays[i],LOW);
//          msWait[i]=millis();
//      }      

if(tripFlag[i]==true  )
{
  if(millis()- msWait[i] >= waitOut)
  {  
    digitalWrite(relays[i],HIGH);
    tripFlag[i]=false;
}
  }
if(tripFlag[i]==false  && currents[i] <= preset[i]) {msWaitout[i]=millis();}
//      
//      if(millis()-msWait[i] >= waitOut)
//     {
//         digitalWrite(relays[i],HIGH);
//          tripFlag[i] =false;
//      } 
    
    
    
    } 

   
  if (digitalRead(OK)== false)
  {  
     beep(1,50);
     setCurrent(page);
    }

}
void readCurrent()
{ 
  float i0=0;
  float y=0;
  //CHANNEL 0 
  digitalWrite(s0,LOW);
  digitalWrite(s1,LOW);
  digitalWrite(s2,LOW);
  for(int j=0;j<10;j++)
  {
  ct.read(&ct0);
  i0 += ct0.amps();
  delayMicroseconds(10);
  }

  y = i0/10;
  y-=0.034;
  if (y<0)y=0;
currents[0]=y;
delayMicroseconds(10);
i0=0;
// CHANNEL 1
  digitalWrite(s0,HIGH);
  digitalWrite(s1,LOW);
  digitalWrite(s2,LOW);
  for(int j=0;j<10;j++)
  {
  ct.read(&ct0);
  i0 += ct0.amps();
  delayMicroseconds(10);
  }

   y = i0/10;
  y-=0.034;
  if (y<0)y=0;
currents[1]=y;
delayMicroseconds(10);
i0=0;

// CHANNEL 2
  digitalWrite(s0,LOW);
  digitalWrite(s1,HIGH);
  digitalWrite(s2,LOW);
  for(int j=0;j<10;j++)
  {
  ct.read(&ct0);
  i0 += ct0.amps();
  delayMicroseconds(10);
  }

   y = i0/10;
  y-=0.034;
  if (y<0)y=0;
currents[2]=y;
delayMicroseconds(10);
i0=0;

// CHANNEL 3
  digitalWrite(s0,HIGH);
  digitalWrite(s1,HIGH);
  digitalWrite(s2,LOW);
  for(int j=0;j<10;j++)
  {
  ct.read(&ct0);
  i0 += ct0.amps();
  delayMicroseconds(10);
  }

   y = i0/10;
  y-=0.034;
  if (y<0)y=0;
currents[3]=y;
delayMicroseconds(10);
i0=0;

// CHANNEL 4
  digitalWrite(s0,LOW);
  digitalWrite(s1,LOW);
  digitalWrite(s2,HIGH);
  for(int j=0;j<10;j++)
  {
  ct.read(&ct0);
  i0 += ct0.amps();
  delayMicroseconds(10);
  }

   y = i0/10;
  y-=0.034;
  if (y<0)y=0;
currents[4]=y;
delayMicroseconds(10);
i0=0;
}

  
 

void beep(int count, int d)
{
 for(int i=0;i<count;i++)
{
 digitalWrite(buzzer,HIGH);
  delay(d); 
 digitalWrite(buzzer,LOW);
 delay(80);
  }
 digitalWrite(buzzer,LOW);
  }
  
void setCurrent(int c)
{
  while(digitalRead(OK) == false)
{;}
  delay(10);
  busy=true;
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print("Set Current");
  lcd.setCursor(0,1);
  lcd << F("Channel: ") << (c+1) ;
 // lcd.print("Channel:");
   setCurr=getCurrent(c)*1000;
  int lastCurr=0;
  setFlag=true;
  while(1)
  {
  lcd.setCursor(0,2);
  if(lastCurr != setCurr)
  {
     lcd.setCursor(0,2);
     lcd.print(space);
     lcd.setCursor(0,2);
     lastCurr = setCurr;
    }
  lcd.print("Current:");
  lcd.print(setCurr);
  lcd.print(" mA  ");

  if(digitalRead(OK) == false)
  {break;}
    }
   busy=false; 
   setFlag=false; 
   long rem=0;
    int mult=0;
    rem=setCurr; 
    while(1)
    {
      if (rem >255)
      {
      rem -= 255;
      mult +=1;
      }
      if (rem <= 255)
      {
        break;
      }
      }
       
    EEPROM.update(remAddress[c], rem);
    EEPROM.update(multAddress[c], mult);
    preset[c]=getCurrent(c);
    lcd.clear();
}
  
float getCurrent(int c)
{
  float val=0;
  float price;
  int rem=0;
   int mult=0; 
   rem  = EEPROM.read(remAddress[c]);
   mult = EEPROM.read(multAddress[c]);
   price =(mult*255)+rem;
   val=price/1000.00;
   beep(1,150); 
   return val;
  } 

ISR(PCINT2_vect) 
 {
  unsigned char result = r.process();
  if (result == DIR_NONE ) {}
  else if (result == DIR_CW && setFlag==true ) {
setCurr+=10;
if(setCurr>9000)setCurr=0;
  }
 
  else if (result == DIR_CCW && setFlag==true ) {
  setCurr-=10;
  if(setCurr<0)setCurr=9000;

    }
 if (result == DIR_CW && saveFlag==true ) {
  yes= !yes;
  }
 
  else if (result == DIR_CCW && saveFlag==true ) {
  yes= !yes;
    }

if (result == DIR_CW && timeFlag==true ) {
  t1 += 1;
   if(t1>60)t1=0;
  }
 
  else if (result == DIR_CCW && timeFlag==true ) {
  t1 -= 1;
    if(t1<0)t1=60;

    }
if (result == DIR_CW && busy==false ) {
  page += 1;
   if(page>4) page=0;
  }

else if (result == DIR_CCW && busy==false ) {
  page -= 1;
    if(page<0)page=4;  
 
}
}
void settings()
{ 
for(int i=0;i<5;i++)
{
  setCurrent(i);
}
  bool lastYes=true;
  lcd.clear();
  saveFlag=true;
while(1){
  lcd.setCursor(3,0);
  lcd.print("Backlight ?");
  lcd.setCursor(3,1);
  lcd.print("NO");
  lcd.setCursor(3,2);
  lcd.print("YES");
  if (lastYes != yes)
  {
    lcd.clear();
    lastYes=yes;
    }
  if(!yes){
  lcd.setCursor(0,1);
  lcd.print(">");
  }
  else if(yes)
  {
  lcd.setCursor(0,2);
  lcd.print(">");
  }
  if(digitalRead(OK) == false)
  {
    break;
    }
    
    if(yes){lcd.backlight();
    x=1;
    }
    else {
      lcd.noBacklight();
      x=0;
      }
  }
  while(digitalRead(OK) == false){;}
    delay(10);
  saveFlag=false;
  EEPROM.update(bk_address, x);
  timeFlag=true;
  int lastT1=0;
  t1=EEPROM.read(to_address);
  lcd.clear();
  while(1)
  {
  lcd.setCursor(0,0);
  lcd.print("Set Wait Timeout (S)"); 
  lcd.setCursor(9,1);
  if(lastT1 != t1)
  {
     lcd.setCursor(0,1);
     lcd.print(space);
     lcd.setCursor(9,1);
     lastT1 = t1;
  }
  lcd.print(t1);
  lcd.print(" S");  
    
if(digitalRead(OK) == false)
  {break;}
    }
   timeFlag=false; 
   EEPROM.update(to_address, t1);
   waitOut = t1*1000;
   useDefault =false;
  }
