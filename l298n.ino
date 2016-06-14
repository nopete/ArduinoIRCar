#include <IR_RC.h>
#include <IR_RCInt.h>

/**
* Bruno Santos, 2013
* feiticeir0@whatgeek.com.pt
* Small code to test DC motors - 2x with a L298 Dual H-Bridge Motor Driver
* Free to share
**/
//Testing the DC Motors
//Pins 5 and 6: controlled by timer0 (8 bits)
//Pins 9 and 10: controlled by timer1 (16 bits)
//Pins 11 and 3: controlled by timer2 (8 bits)
// RC library uses timer2 and clears it hence the motor PWM must not use pins 11 and 3
//Define Pins
//Motor A
int enableA = 5;
int pinA1 = 4;
int pinA2 = 2;
//Motor B
int enableB = 6;
int pinB1 = 8;
int pinB2 = 7;

int RECV_PIN = 12;
IRrecv irrecv(RECV_PIN);
decode_results results;

int LED = 13;

int HIGH_LIMIT=255;
int LOW_LIMIT=90;
int STEP=15;

#define UP    0x807F18E7
#define DOWN  0x807F629D
#define LEFT  0x807F827D
#define RIGHT 0x807F926D
#define KEEP  0xFFFFFFFF

#define VOL_UP 0x807F02FD
#define VOL_DOWN 0x807F22DD
#define CH_UP 0x807FC03F
#define CH_DOWN 0x807FE01F

#define OK 0x807FA25D
#define POWER 0x807FFA05;

void pwm_go(int val, bool A, bool B)
{
  if (A)
  {
    if (B)
      Serial.print("BOTH ");
     else
      Serial.print("A ");
  }
  else
  {
    if (B)
      Serial.print("B ");
     else
     {
      Serial.println("NONE");
      return;
     }
  }
  Serial.println(val);
//delay(100);return;
  if (val > 0)
  {
    if (A)
    {
      analogWrite(enableA, val);
      digitalWrite(pinA1, HIGH);  
      digitalWrite(pinA2, LOW);  
    }
    if (B)
    {
      analogWrite(enableB, val);
      digitalWrite(pinB1, LOW);  
      digitalWrite(pinB2, HIGH);      
    }
    //delay(20);
  }
  else if (val < 0)
  {
    if (A)
    {
      analogWrite(enableA, -val);
      digitalWrite(pinA1, LOW);  
      digitalWrite(pinA2, HIGH);  
    }
    if (B)
    {
      analogWrite(enableB, -val);      
      digitalWrite(pinB1, HIGH);  
      digitalWrite(pinB2, LOW);      
    }
//digitalWrite(enableA, HIGH);  
//digitalWrite(enableB, HIGH);
    //delay(20);
  }
  else
  {
    if (A)
    {
      digitalWrite(pinA2, LOW);  
      digitalWrite(pinA1, LOW);  
//    digitalWrite(enableA, LOW);  
//    analogWrite(enableA, 0);
    }
    if (B)
    {
      digitalWrite(pinB2, LOW);  
      digitalWrite(pinB1, LOW);
//    digitalWrite(enableB, LOW);
//    analogWrite(enableB, 0);
    }
  }

}

int rpm = 0;
int last_inc = 0;

void Test(bool A, bool B)
{
  for (int i=LOW_LIMIT; i <= HIGH_LIMIT; i += STEP)
  {
    pwm_go(i, A, B);
    delay(1000);
  }
  pwm_go(0, A, B);
  for (int i=LOW_LIMIT; i <= HIGH_LIMIT; i += STEP)
  {
    pwm_go(-i, A, B);
    delay(1000);
  }
  pwm_go(0, A, B);
  
  for (int i=0; i < 2; ++i)
  {
    digitalWrite(LED,HIGH);
    delay(500);
    digitalWrite(LED,LOW);
    delay(500);
  }
}

void setup()
{
  Serial.begin (9600);

  //configure pin modes
  pinMode(LED, OUTPUT);
  
//  pinMode (enableA, OUTPUT);
  pinMode (pinA1, OUTPUT);
  pinMode (pinA2, OUTPUT);
//  pinMode (enableB, OUTPUT);
  pinMode (pinB1, OUTPUT);
  pinMode (pinB2, OUTPUT);

  Test(true, true);
  
  rpm = 0;
  last_inc = 0;
  irrecv.enableIRIn(); // Start the receiver

  Serial.println("Ready");

#ifdef IR_USE_TIMER1
  Serial.println("IR_USE_TIMER1");
#endif
#ifdef IR_USE_TIMER2
  Serial.println("IR_USE_TIMER2");
#endif
#ifdef IR_USE_TIMER3
  Serial.println("IR_USE_TIMER3");
#endif
#ifdef IR_USE_TIMER4
  Serial.println("IR_USE_TIMER4");
#endif
#ifdef IR_USE_TIMER5
  Serial.println("IR_USE_TIMER5");
#endif
#ifdef IR_USE_TIMER6
  Serial.println("IR_USE_TIMER6");
#endif

  Serial.print("PWM_PIN=");
  Serial.println(TIMER_PWM_PIN);
}


void loop_pwmCheck()
{
  unsigned long decCode=0;
  if(irrecv.decode(&results))
  {  
    decCode = results.value;
    //Serial.println("CODE1");
    Serial.println(decCode, HEX);
    Serial.println("\t\t\t == \t\tCODE");
    if (results.overflow)
    {
       Serial.println("OVERFLOW");
       irrecv.resume();
       return;
    }
    
    switch (decCode)
    {
      case  VOL_UP:
        Serial.println("VOL_UP");
        last_inc = STEP;
        break;
      case  VOL_DOWN:
        Serial.println("VOL_DOWN");
        last_inc = -STEP;
        break;
      case OK:
        Serial.println("OK");
        last_inc = 0;
        rpm = 0;
        pwm_go(0, true, true);
        break;
      case KEEP:
        //Serial.println("KEEP");
        break;
      default:
        last_inc = 0;
        break;
    }

    if (last_inc != 0)
    {
      rpm += last_inc;
      if (rpm > HIGH_LIMIT)
        rpm = HIGH_LIMIT;
      else if (rpm < -HIGH_LIMIT)
        rpm = -HIGH_LIMIT;
      else
      {
        if (rpm > 0 && rpm < LOW_LIMIT)
          rpm = last_inc > 0 ? LOW_LIMIT : 0;
        if (rpm < 0 && rpm > -LOW_LIMIT)
          rpm = last_inc < 0 ? -LOW_LIMIT : 0;

        pwm_go(rpm, true, true);
      }
    }
//    else
    {
      delay(100);
    }
    irrecv.resume();
  }
}

void loop()
{
  loop_pwmCheck();
}

