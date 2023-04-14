#include <Arduino.h>
#include <FS.h>
#include <TFT_eSPI.h>              // Hardware-specific library
#include <TFT_eWidget.h>           // Widget library
#include "Free_Fonts.h" // Include the header file attached to this sketch
#include <main.h>

//stepper related----------------START---------------
#define DIR_PIN 15 //direction set pin GPIO 2
#define STEP_PIN 2 //GPIO 3-pulse pin

#define CLOCK_WISE 1
#define COUNTER_CLOCK_WISE 0

#define RUN_STEPPER 1
#define STOP_STEPPER 0

struct stepper
{
  int set_motor;
  int set_dir;
}stepper;
//stepper related----------------END------------------

//-------------------------------TIMER START----------------------
struct timer
{
  int set_hour;
  int set_minute;
  int set_second;

  int running_hour;
  int running_minute;
  int running_second;

  int sec_cntr;
  int min_cntr;
  int hour_cntr;

}timer;

struct Global_Flags
{
  int _1sec_flag;
  int _1min_flag;
  int _1hour_flag;
}global_flags;

char text_value[2];
hw_timer_t *My_timer = NULL;
//-------------------------------Timer STOP-----------------------
TFT_eSPI tft = TFT_eSPI();         // Invoke custom library

ButtonWidget start_or_stop_butn = ButtonWidget(&tft);
ButtonWidget cw_butn = ButtonWidget(&tft);
ButtonWidget ccw_butn = ButtonWidget(&tft);
ButtonWidget both_butn = ButtonWidget(&tft);

// Create an array of button instances to use in for() loops
// This is more useful where large numbers of buttons are employed
ButtonWidget* btn[] = {&start_or_stop_butn, &cw_butn, &ccw_butn, &both_butn};
uint8_t buttonCount = sizeof(btn) / sizeof(btn[0]);

void stepper_motor_stop(void);
void stepper_motor_start(void);
void TFT_Init(void);
void run_stepper(void);
void initButtons(void);
void initTexts(void);
void start_or_stop_press(void);
void cw_butn_press(void);
void ccw_butn_press(void);
void Timer_Init(void);
void display_number(u_int8_t value, int32_t x, int32_t y, int32_t w, int32_t h, uint16_t bg_color, uint16_t text_color);

void setup() {
  Serial.begin(115200);
  
  //stepper motor pins
  pinMode(DIR_PIN,OUTPUT);
  pinMode(STEP_PIN,OUTPUT);

  digitalWrite(DIR_PIN,CLOCK_WISE);
  stepper.set_dir = CLOCK_WISE;

  TFT_Init();
  
  initButtons();

  initTexts();
}

void loop() {
  static uint32_t scanTime = millis();
  uint16_t t_x = 9999, t_y = 9999; // To store the touch coordinates

  // Scan keys every 50ms at most
  if (millis() - scanTime >= 50) {
    // Pressed will be set true if there is a valid touch on the screen
    bool pressed = tft.getTouch(&t_x, &t_y);
    scanTime = millis();
    for (uint8_t b = 0; b < buttonCount; b++) {
      if (pressed) {
        if (btn[b]->contains(t_x, t_y)) {
          btn[b]->press(true);
          btn[b]->pressAction();
        }
      }
      else {
        btn[b]->press(false);
        btn[b]->releaseAction();
      }
    }
  }

  //stepper
  if(stepper.set_motor == RUN_STEPPER)
  {
    if(digitalRead(DIR_PIN) != stepper.set_dir)
    {
      digitalWrite(DIR_PIN,stepper.set_dir);
    }
    run_stepper();
  }
}

void TFT_Init(void)
{
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setFreeFont(FF18);
}

void initTexts(void)
{
  //rotations head
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Duration",40,140);
  tft.drawString("Rotations",40,40);
  timer.sec_cntr = 0;
  timer.min_cntr = 5;
  timer.hour_cntr = 0;
  display_number(timer.sec_cntr, 120, 185, 40, 40, TFT_BLACK, TFT_WHITE);
  display_number(timer.min_cntr, 78, 185, 40, 40, TFT_BLACK, TFT_WHITE);
  display_number(timer.hour_cntr, 36, 185, 40, 40, TFT_BLACK, TFT_WHITE);
}

//run stepper motor
void run_stepper(void)
{
  digitalWrite(STEP_PIN,HIGH);
  delayMicroseconds(500);
  digitalWrite(STEP_PIN,LOW);
  delayMicroseconds(500);
}

//initializing - creating all the buttons
void initButtons(void) {
  //start button
  start_or_stop_butn.initButtonUL(204,165,110,70,TFT_WHITE,TFT_GREEN,TFT_RED,"START",1);
  start_or_stop_butn.setPressAction(start_or_stop_press);
  start_or_stop_butn.drawSmoothButton(false, 2, TFT_BLACK);
  //clock wise button
  cw_butn.initButtonUL(151,9,68,48,TFT_WHITE,TFT_BLACK,TFT_GREEN,"CW",1);
  cw_butn.setPressAction(cw_butn_press);
  cw_butn.drawSmoothButton(false, 2, TFT_BLACK);
  cw_butn.drawSmoothButton(!cw_butn.getState(), 3, TFT_BLACK, cw_butn.getState() ? "CW" : "CW");
  //counter clock wise
  ccw_butn.initButtonUL(246,9,68,48,TFT_WHITE,TFT_BLACK,TFT_GREEN,"CCW",1);
  ccw_butn.setPressAction(ccw_butn_press);
  ccw_butn.drawSmoothButton(false, 2, TFT_BLACK);
  //both direction buttons
  //code pending
}

//start/stop button press action
void start_or_stop_press(void)
{
  if (start_or_stop_butn.justPressed()) 
  {
    start_or_stop_butn.drawSmoothButton(!start_or_stop_butn.getState(), 3, TFT_BLACK, start_or_stop_butn.getState() ? "START" : "STOP");
    if (start_or_stop_butn.getState())
    {
      stepper_motor_start();
    }
    else
    {
      stepper_motor_stop();
    }
  }
}

void cw_butn_press(void) {
  if(cw_butn.justPressed()) {
    cw_butn.drawSmoothButton(!cw_butn.getState(), 3, TFT_BLACK, cw_butn.getState() ? "CW" : "CW");
    ccw_butn.drawSmoothButton(!ccw_butn.getState(), 3, TFT_BLACK, ccw_butn.getState() ? "CCW" : "CCW");
    if (cw_butn.getState())
    {
      stepper.set_dir = CLOCK_WISE;
    }
    else
    {
      stepper.set_dir = COUNTER_CLOCK_WISE;
    }
  }
}

void ccw_butn_press(void) {
  if(ccw_butn.justPressed()) {
    ccw_butn.drawSmoothButton(!ccw_butn.getState(), 3, TFT_BLACK, ccw_butn.getState() ? "CCW" : "CCW");
    cw_butn.drawSmoothButton(!cw_butn.getState(), 3, TFT_BLACK, cw_butn.getState() ? "CW" : "CW");
    if (ccw_butn.getState())
    {
      stepper.set_dir = COUNTER_CLOCK_WISE;
    }
    else
    {
      stepper.set_dir = CLOCK_WISE;
    }
  }
}

void IRAM_ATTR onTimer(){
  timer.sec_cntr--;
  if(timer.sec_cntr < 0)
  {
    timer.sec_cntr = 59;
    timer.min_cntr--;
    if(timer.min_cntr < 0)
    {
      timer.min_cntr = 59;
      timer.hour_cntr--;
      if(timer.hour_cntr < 0)
      {
        timer.hour_cntr = RESET;
        timer.sec_cntr = RESET;
        timer.min_cntr = RESET;
        stepper_motor_stop();
      }
    }
  }
  display_number(timer.sec_cntr, 120, 185, 40, 40, TFT_BLACK, TFT_WHITE);
  display_number(timer.min_cntr, 78, 185, 40, 40, TFT_BLACK, TFT_WHITE);
  display_number(timer.hour_cntr, 36, 185, 40, 40, TFT_BLACK, TFT_WHITE);
}

void Timer_Init(void)
{
  //initializing 1 sec interrupt
  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 1000000, true);
  timerAlarmEnable(My_timer);
}

void Timer_Stop(void)
{
  timerDetachInterrupt(My_timer);
  timerAlarmDisable(My_timer);
}

void stepper_motor_start(void)
{
  stepper.set_motor = RUN_STEPPER;
  Timer_Init();
}

void stepper_motor_stop(void)
{
  stepper.set_motor = STOP_STEPPER;
  Timer_Stop();
}

void display_number(u_int8_t value, int32_t x, int32_t y, int32_t w, int32_t h, uint16_t bg_color, uint16_t text_color)
{
  tft.setTextColor(text_color, bg_color);
  tft.fillRect(x,y,w,h,bg_color);
  tft.drawNumber(value,x,y);
  tft.drawString(":",104,183);
  tft.drawString(":",61,183);
}

