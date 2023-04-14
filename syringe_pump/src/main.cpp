#include <Arduino.h>
#include <FS.h>
#include <TFT_eSPI.h>              // Hardware-specific library
#include <TFT_eWidget.h>           // Widget library
#include "Free_Fonts.h" // Include the header file attached to this sketch
#include <main.h>
#include <my_tasks.h>

//stepper related----------------START---------------
#define DIR_PIN 15 //direction set pin GPIO 2
#define STEP_PIN 2 //GPIO 3-pulse pin

#define CLOCK_WISE 1
#define COUNTER_CLOCK_WISE 0

#define RUN_STEPPER 1
#define STOP_STEPPER 0

#define MAX_SEC 59
#define MAX_MIN 59
#define MAX_HOUR 23

char buf[2];

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

struct set_button
{
  int set_button_id;
}set_button;

struct Global_Flags
{
  int _1sec_flag;
  int _1min_flag;
  int _1hour_flag;

  int timer_start;

  bool set_butn_clicked;

  bool both_dir_selected;

  bool start_butn_clicked;
}global_flags;


char text_value[2];
hw_timer_t *My_timer = NULL;
//-------------------------------Timer STOP-----------------------
TFT_eSPI tft = TFT_eSPI();         // Invoke custom library

ButtonWidget start_or_stop_butn = ButtonWidget(&tft);
ButtonWidget cw_butn = ButtonWidget(&tft);
ButtonWidget ccw_butn = ButtonWidget(&tft);
ButtonWidget both_butn = ButtonWidget(&tft);
ButtonWidget set_butn = ButtonWidget(&tft);

ButtonWidget add_butn = ButtonWidget(&tft);
ButtonWidget sub_butn = ButtonWidget(&tft);


// Create an array of button instances to use in for() loops
// This is more useful where large numbers of buttons are employed
ButtonWidget* btn[] = {&start_or_stop_butn, &cw_butn, &ccw_butn, &both_butn,&set_butn,&add_butn,&sub_butn};
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

void both_butn_pres(void);

void set_butn_pres(void);
void add_butn_pres(void);
void sub_butn_pres(void);

void _2ns_tasks(void);

void load_set_timer(void);

void setup() {
  Serial.begin(115200);
  
  //stepper motor pins
  pinMode(DIR_PIN,OUTPUT);
  pinMode(STEP_PIN,OUTPUT);

  digitalWrite(DIR_PIN,CLOCK_WISE);
  stepper.set_dir = CLOCK_WISE;

  load_set_timer();

  TFT_Init();
  
  initButtons();

  initTexts();

  

  task_setup();
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

  tft.getTouch(&t_x, &t_y);
  if(global_flags.set_butn_clicked==1)
  {
    if(t_x>=85&&t_x<=125&&t_y>=110&&t_y<=150)
    {
      //min button
      set_button.set_button_id=2;
      display_number(timer.min_cntr, 85, 110, 40, 40, TFT_BLACK, TFT_GREEN);
      display_number(timer.hour_cntr, 14, 110, 40, 40, TFT_BLACK, TFT_WHITE);
      display_number(timer.sec_cntr, 150, 110, 40, 40, TFT_BLACK, TFT_WHITE);
    }
    else if(t_x>=14&&t_x<=54&&t_y>=110&&t_y<=150)
    {
      //hour button
      set_button.set_button_id=1;
      display_number(timer.min_cntr, 85, 110, 40, 40, TFT_BLACK, TFT_WHITE);
      display_number(timer.hour_cntr, 14, 110, 40, 40, TFT_BLACK, TFT_GREEN);
      display_number(timer.sec_cntr, 150, 110, 40, 40, TFT_BLACK, TFT_WHITE);
    }
    else if(t_x>=150&&t_x<=194&&t_y>=110&&t_y<=150)
    {
      //sec button
      set_button.set_button_id=3;
      display_number(timer.min_cntr, 85, 110, 40, 40, TFT_BLACK, TFT_WHITE);
      display_number(timer.hour_cntr, 14, 110, 40, 40, TFT_BLACK, TFT_WHITE);
      display_number(timer.sec_cntr, 150, 110, 40, 40, TFT_BLACK, TFT_GREEN);
    }
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
  tft.drawString("Duration",60,70);
  timer.sec_cntr = 0;
  timer.min_cntr = 0;
  timer.hour_cntr = 0;
  tft.setFreeFont(FF20);
  display_number(timer.sec_cntr, 150, 110, 40, 40, TFT_BLACK, TFT_WHITE);
  display_number(timer.min_cntr,85, 110, 40, 40, TFT_BLACK, TFT_WHITE);
  display_number(timer.hour_cntr, 14, 110, 40, 40, TFT_BLACK, TFT_WHITE);
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
  cw_butn.initButtonUL(7,9,68,48,TFT_WHITE,TFT_BLACK,TFT_GREEN,"CW",1);
  cw_butn.setPressAction(cw_butn_press);
  cw_butn.drawSmoothButton(false, 2, TFT_BLACK);
  cw_butn.drawSmoothButton(!cw_butn.getState(), 3, TFT_BLACK, cw_butn.getState() ? "CW" : "CW");
  //counter clock wise
  ccw_butn.initButtonUL(95,9,68,48,TFT_WHITE,TFT_BLACK,TFT_GREEN,"CCW",1);
  ccw_butn.setPressAction(ccw_butn_press);
  ccw_butn.drawSmoothButton(false, 2, TFT_BLACK);
  //set button 
  set_butn.initButtonUL(221,115,78,43,TFT_WHITE,TFT_BLACK,TFT_GREEN,"SET",1);
  set_butn.setPressAction(set_butn_pres);
  set_butn.drawSmoothButton(false, 2, TFT_BLACK);
  //both_butn 
  both_butn.initButtonUL(211,16,78,43,TFT_WHITE,TFT_BLACK,TFT_GREEN,"BOTH",1);
  both_butn.setPressAction(both_butn_pres);
  both_butn.drawSmoothButton(false, 2, TFT_BLACK);
}

//start/stop button press action
void start_or_stop_press(void)
{
  tft.setFreeFont(FF18);
  if (start_or_stop_butn.justPressed()) 
  {
    start_or_stop_butn.drawSmoothButton(!start_or_stop_butn.getState(), 3, TFT_BLACK, start_or_stop_butn.getState() ? "START" : "STOP");
    if (start_or_stop_butn.getState())
    {
      global_flags.start_butn_clicked=1;
      if(global_flags.set_butn_clicked==1)
      {
        set_butn.drawSmoothButton(!set_butn.getState(), 3, TFT_BLACK, set_butn.getState() ? "SET" : "SET");
      }
      global_flags.set_butn_clicked=0;
      tft.fillRect(50,178,120,50,TFT_BLACK);
      stepper_motor_start();
      load_set_timer();
    }
    else
    {
      stepper_motor_stop();
      global_flags.start_butn_clicked=0;
    }
  }
}

void cw_butn_press(void) {
  tft.setFreeFont(FF18);
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
  tft.setFreeFont(FF18);
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
  if(global_flags.timer_start==1)
  {
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
        tft.setFreeFont(FF18);
        if(global_flags.both_dir_selected==1)
        {
          timer.sec_cntr=timer.set_second;
          timer.min_cntr=timer.set_minute;
          timer.hour_cntr=timer.set_hour;
          cw_butn.drawSmoothButton(!cw_butn.getState(), 3, TFT_BLACK, cw_butn.getState() ? "CW" : "CW");
          ccw_butn.drawSmoothButton(!ccw_butn.getState(), 3, TFT_BLACK, ccw_butn.getState() ? "CCW" : "CCW");
          stepper.set_dir=~stepper.set_dir;
        }
        else
        {
        global_flags.start_butn_clicked=0;
        start_or_stop_butn.drawSmoothButton(!start_or_stop_butn.getState(), 3, TFT_BLACK, start_or_stop_butn.getState() ? "START" : "STOP");
        stepper_motor_stop();
        }
      }
    }
  }
  display_number(timer.sec_cntr, 150, 110, 40, 40, TFT_BLACK, TFT_WHITE);
  display_number(timer.min_cntr, 85, 110, 40, 40, TFT_BLACK, TFT_WHITE);
  display_number(timer.hour_cntr, 14, 110, 40, 40, TFT_BLACK, TFT_WHITE);
  }
}

void Timer_Init(void)
{
  //initializing 1 sec interrupt
  global_flags.timer_start=1;
  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 1000000, true);
  timerAlarmEnable(My_timer);
}

void Timer_Stop(void)
{
  if(global_flags.timer_start==1)
  {
    timerDetachInterrupt(My_timer);
    timerAlarmDisable(My_timer);
  }
}

void stepper_motor_start(void)
{
  stepper.set_motor = RUN_STEPPER;
  if(timer.sec_cntr==0&&timer.min_cntr==0&&timer.hour_cntr==0)
  {
    //do nothing
    global_flags.timer_start=0;
  }
  else{
    Timer_Init();
  }
}

void stepper_motor_stop(void)
{
  stepper.set_motor = STOP_STEPPER;
  global_flags.timer_start=0;
  Timer_Stop();
}

void display_number(u_int8_t value, int32_t x, int32_t y, int32_t w, int32_t h, uint16_t bg_color, uint16_t text_color)
{
  tft.setFreeFont(FF20);
  tft.setTextColor(text_color, bg_color);
  tft.fillRect(x,y,w+5,h,bg_color);
  if(value%10==1)
  {
    tft.fillRect(x+5,y,w+5,h,bg_color);
  }
  if(value<10)
  {
    sprintf(buf, "0%d", value);
  }
  else
  {
   sprintf(buf, "%d", value);
  }
  tft.drawString(buf,x,y,1);
  tft.drawString(":",134,107);
  tft.drawString(":",71,107);
}

void set_butn_pres(void)
{
  tft.setFreeFont(FF18);
  if (set_butn.justPressed()&&global_flags.start_butn_clicked==0) 
  {
    set_butn.drawSmoothButton(!set_butn.getState(), 3, TFT_BLACK, set_butn.getState() ? "SET" : "SET");
    if (set_butn.getState())
    {
      load_set_timer();
      global_flags.set_butn_clicked=1;
      set_button.set_button_id=1;
      display_number(timer.hour_cntr, 14, 110, 40, 40, TFT_BLACK, TFT_GREEN);
      display_number(timer.min_cntr, 85, 110, 40, 40, TFT_BLACK, TFT_WHITE);
      display_number(timer.sec_cntr, 150, 110, 40, 40, TFT_BLACK, TFT_WHITE);
      add_butn.initButtonUL(54,180,40,40,TFT_WHITE,TFT_BLACK,TFT_GREEN,"+",1);
      add_butn.setPressAction(add_butn_pres);
      add_butn.drawSmoothButton(false, 1, TFT_BLACK);
      sub_butn.initButtonUL(114,180,40,40,TFT_WHITE,TFT_BLACK,TFT_GREEN,"-",1);
      sub_butn.setPressAction(sub_butn_pres);
      sub_butn.drawSmoothButton(false, 1, TFT_BLACK);
    }
    else
    {
      global_flags.set_butn_clicked=0;
      tft.fillRect(50,178,120,50,TFT_BLACK);
    }
  }
}


void add_butn_pres(void)
{
  if (add_butn.justPressed()) 
  {
    if(set_button.set_button_id==1)
    {
      timer.hour_cntr++;
      if(timer.hour_cntr>23)
      {
        timer.hour_cntr=23;
      }
      display_number(timer.hour_cntr, 14, 110, 40, 40, TFT_BLACK, TFT_GREEN);
    }
    else if(set_button.set_button_id==2)
    {
      timer.min_cntr++;
      if(timer.min_cntr>59)
      {
        timer.min_cntr=59;
      }
      display_number(timer.min_cntr, 85, 110, 40, 40, TFT_BLACK, TFT_GREEN);
    }
    else if(set_button.set_button_id==3)
    {
      timer.sec_cntr++;
      if(timer.sec_cntr>59)
      {
        timer.sec_cntr=59;
      }
      display_number(timer.sec_cntr, 150, 110, 40, 40, TFT_BLACK, TFT_GREEN);
    }
  }
}

void sub_butn_pres(void)
{
  if (sub_butn.justPressed()) 
  {
    if(set_button.set_button_id==1)
    {
      timer.hour_cntr--;
      if(timer.hour_cntr<0)
      {
        timer.hour_cntr=0;
      }
      display_number(timer.hour_cntr, 14, 110, 40, 40, TFT_BLACK, TFT_GREEN);
    }
    else if(set_button.set_button_id==2)
    {
      timer.min_cntr--;
      if(timer.min_cntr<0)
      {
        timer.min_cntr=0;
      }
      display_number(timer.min_cntr, 85, 110, 40, 40, TFT_BLACK, TFT_GREEN);
    }
    else if(set_button.set_button_id==3)
    {
      timer.sec_cntr--;
      if(timer.sec_cntr<0)
      {
        timer.sec_cntr=0;
      }
      display_number(timer.sec_cntr, 150, 110, 40, 40, TFT_BLACK, TFT_GREEN);
    }
  }
}

void _2ns_tasks(void)
{
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

void both_butn_pres(void)
{
  tft.setFreeFont(FF18);
  if(both_butn.justPressed()) {
  both_butn.drawSmoothButton(!both_butn.getState(), 3, TFT_BLACK, both_butn.getState() ? "BOTH" : "BOTH");
  if(both_butn.getState())
  {
    global_flags.both_dir_selected=1;
  }
  else
  {
    global_flags.both_dir_selected=0;
  }
  }
}

void load_set_timer(void)
{
  timer.set_second=timer.sec_cntr;
  timer.set_minute=timer.min_cntr;
  timer.set_hour=timer.hour_cntr;
}