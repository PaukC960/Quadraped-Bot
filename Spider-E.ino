// head_up and head_down test with z_default
// tunr_left not finished

#include <FlexiTimer2.h>
#include <Servo.h>
#include <SoftwareSerial.h>

// Set up a new serial port for the HM-10
// Pin 13 = RX (Connect to HM-10 TX)
// Pin 14 = TX (Connect to HM-10 RX)
SoftwareSerial HM10(13, 14); 

char data = 0;         // Variable to store incoming Bluetooth data

Servo servo[4][3];    // Servo objects

// define servo ports from Arduino
const int servo_pin[4][3] = { {2,3,4}, {5,6,7}, {8,9,10}, {11,12,13} };

/* Size of the robot (larger than the reference bot)--------------------------*/ 
const float length_a = 63;
const float length_b = 82.5;
const float length_c = 50;
const float length_side = 84;
const float z_absolute = -28;

/* Constants for movement ------------------------------------------------- */ 
const float x_default = 80, x_offset = 5;
const float y_start = 0, y_step = 30;
const float z_default = -50, z_up = -30, z_boot = z_absolute;
//const float y_default = x_default;  // --- dont know why

/* Variables for movement -----------------------------------------------------*/ 
volatile float site_now[4][3];      // coordinates of the tip of each leg with respect to the body-coax joint
volatile float site_expect [4][3];  // expected coordinates for the tip of the leg to move to
float temp_speed[4][3];             // each axis' speed, needs to be recalculated before each movement
float move_speed;
float speed_multiple = 1;           // --- if one, why matters?
const float spot_turn_speed = 4;
const float leg_move_speed = 8;     // for go-forward/ go-backward, first leg moves then, the body follows
const float body_move_speed = 3;
const float stand_sit_speed = 1;
volatile int rest_counter;          // ---- +1/ 0.02 s for automatic rest

// function's parameter 
const float KEEP = 255;             // parameter to pass for the stationary joint

// define Pi 
const float pi = 3.1415926;

/* Constants for turn ----------------------------------------------------*/
// temp lengths
const float temp_a = sqrt ( pow (2 * x_default + length_side, 2) + pow (y_step, 2) )
const float temp_b = 2 * (y_start + y_step) + length_side;
const float temp_c = sqrt ( pow (2 * x_default + length_side, 2) + pow ( 2 * y_start + y_step + length_side , 2 ) )
const float temp_alpha = acos ( pow( temp_a,2 ) + pow( temp_b,2 ) - pow( temp_c, 2)) / 2 / temp_a / temp_b);

// site for turn
const float turn_x1 = (temp_a - length_side) / 2;
const float turn_y1 = y_start + y_step / 2;
const float turn_x0 = turn_x1 - temp_b * cos(temp_alpha);
const float turn_y0 = temp_b * sin (temp_alpha) - turn_y1 - length_side;
/* --------------------------------------------------------------------------*/


void setup() {
  Serial.begin(9600);

  // Start software serial for HM-10 Bluetooth
  HM10.begin(9600); 
  Serial.println ("Spider bot activated!");

  // default position at the start
  pinMode(14, OUTPUT);
  set_site (0, x_default - x_offset, y_start + y_step, z_boot);
  set_site (1, x_default - x_offset, y_start + y_step, z_boot);
  set_site (2, x_default + x_offset, y_start, z_boot);
  set_site (3, x_default + x_offset, y_start, z_boot);

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++) {
      site_now[i][j] = site_expect[i][j];
    }
  }

  // start running FlexiTimer2 and servo_service
  FlexiTimer2::set (20, servo_service);     // run servo_service every 20 millisec
  FlexiTimer2::start();

  Serial.println ("Servo service initiatied!");

  //initialize servos
  servo_attach();
  Serial.println ("Servos warmed up!");
  Serial.println ("Robot initialization complete");
}

void loop() {
  // Check if the HM-10 has received command
  if(HM10.available() > 0)      
  {
    data = HM10.read();        
    
    // Print what we received to the Serial Monitor for debugging
    Serial.print("Received Command: ");
    Serial.println(data);        

    if(data == 'F' || data == 'f') 
    { 
      Serial.println("Step forward");
      step_forward();
    }
    else if(data == 'B' || data == 'b')        
    { 
      Serial.println("Step back");
      step_back();
    }    
    else if(data == 'L' || data == 'l')        
    { 
      Serial.println("Turn left");
      turn_left();
    }
    else if(data == 'R' || data == 'r')        
    { 
      Serial.println("Turn right");
      turn_right();
    } 
    else if(data == 'X' || data == 'x')
    {
      Serial.println("Stand");
      stand();
    }
    else if(data == 'Y' || data == 'y')
    {
      Serial.println("Sit");
      sit();   
    }
    else if(data == 'S' || data == 's')        
    { 
      Serial.println ("Hand Shake");
      hand_shake(3);
    }
    // LEDs headlight
    else if(data == 'O')        
    { 
      digitalWrite(14, HIGH);
    } 
    else if(data == 'o')        
    { 
      digitalWrite(14, LOW);
    }
    else if(data == 'W' || data == 'w')        
    { 
      Serial.println("Hand wave");
      hand_wave(3);
    } 
    else if(data == 'D' || data = 'd')        
    { 
      Serial.println("Body dance");
      body_dance(10);
    } 

    while(HM10.available()) { HM10.read(); }
  }
}

void servo_attach() {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++) {
      servo[i][j].attach(servo_pin[i][j]);
      delay (100);
    }
  }
}

void servo_detach() {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++) {
      servo[i][j].detach();
      delay (100);
    }
  }
}

void sit() {
  move_speed = stand_sit_speed;
  for ( int leg = 0; leg < 4; leg++ ) {
    set_site(leg, KEEP, KEEP, z_boot);
  }
  wait_all_reach();
}

void stand() {
  move_speed = stand_sit_speed;
  for (int leg = 0; leg < 4; leg++ ) {
    set_site(leg, KEEP, KEEP, z_default);
  }
  wait_all_reach();
}

void step_forward() {

  move_speed = leg_move_speed;

  if (site_now[2][1] == y_start) {
    // leg 2 & 1 move

    /* walking gait ----------------------------------------------------
    set_up position
    leg_0 & _1 ---> y_step
    leg_2 & _3 ---> y_0
    (leg 0 & 2 are front)

    first step ------------------------
    leg_2 steps to 2 * y_step

    body_push -------------------------
    leg_0 (pull) y_step -> y_0              - front leg
    leg_1 (push) y_step -> 2 * y_step       - back
    leg_2 (pull) 2 * y_step -> y_step       - front
    leg_3 (push) y_0 -> y_step              - back

    leg_1 catch up --------------------
    leg_1 lifts up, move from 2 * y_step -> y_0
    
    */ 

    set_site (2, x_default + x_offset, y_start, z_up);
    wait_all_reach();
    set_site (2, x_default + x_offset, y_start + 2 * y_step, z_up);
    wait_all_reach();
    set_site (2, x_default + x_offset, y_start + 2 * y_step, z_default);
    wait_all_reach();

    move_speed = body_move_speed;
    // body pushes the leg and wait_all_reach() at the end --> all legs move at the same time
    set_site (0, x_default + x_offset, y_start, z_default);
    set_site (1, x_default + x_offset, y_start + 2 * y_step, z_default);
    set_site (2, x_default - x_offset, y_start + y_step, z_default);
    set_site (3, x_default - x_offset, y_start + y_step, z_default);
    wait_all_reach();

    // move leg 1
    move_speed = leg_move_speed
    set_site (1, x_default + x_offset, y_start + 2 * y_step, z_up);
    wait_all_reach();
    set_site (1, x_default + x_offset, y_start, z_up);
    wait_all_reach();
    set_site (1, x_default + x_offset, y_start, z_default);
    wait_all_reach();
  }
  else {
    // leg 0 & 3 move

    
    // leg 0 move first
    move_speed = leg_move_speed;
    set_site (0, x_default + x_offset, y_start, z_up);
    wait_all_reach();
    set_site (0, x_default + x_offset, y_start + 2 * y_step, z_up);
    wait_all_reach();
    set_site (0, x_default + x_offset, y_start + 2 * y_step, z_default);
    wait_all_reach();

    // push the body 
    move_speed = body_move_speed;

    set_site (0, x_default - x_offset, y_start + y_step, z_default);
    set_site (1, x_default - x_offset, y_start + y_step, z_default);
    set_site (2, x_default + x_offset, y_start, z_default);
    set_site (3, x_default + x_offset, y_step + 2 * y_step, z_default);
    wait_all_reach();

    // move leg 3 to catch up
    move_speed = leg_move_speed;

    set_site (3, x_default + x_offset, y_start + 2 * y_step, z_up);
    wait_all_reach();
    set_site (3, x_default + x_offset, y_start, z_up);
    wait_all_reach();
    set_site (3, x_default + x_offset, y_start, z_default);
    wait_all_reach();
  }
}

void step_backward() {

  if (site_now[3][1] == y_start) {
    // leg 0 & 3 move

    // leg 3 first
    move_speed = leg_move_speed;
    set_site (3, x_default + x_offset, y_start, z_up);
    wait_all_reach();
    set_site (3, x_default + x_offset, y_start + 2 * y_step, z_up);
    wait_all_reach();
    set_site (3, x_default + x_offset, y_start + 2 * y_step, z_default);
    wait_all_reach();

    // push / pull the body simultaneously (only one wait_all_reach())
    move_speed = body_move_speed;
    set_site (0, x_default + x_offset, y_start + 2  * y_step, z_default);
    set_site (1, x_default - x_offset, y_start, z_default);
    set_site (2, x_default + x_offset, y_start + y_step, z_default);
    set_site (3, x_default - x_offset, y_start + y_step, z_default);
    wait_all_reach();

    // leg 0 catch up
    move_speed = leg_move_speed;
    set_site (0, x_default + x_offset, y_start + 2 * y_step, z_up);
    wait_all_reach();
    set_site (0, x_default + x_offset, y_start, z_up);
    wait_all_reach();
    set_site (0, x_default + x_offset, y_start, z_default);
    wait_all_reach();
  }

  // leg 1 & 2 move
  else {
    move_speed = leg_move_speed;
    // leg 1 first
    set_site (1, x_default + x_offset, y_start, z_up);
    wait_all_reach();
    set_site (1, x_default + x_offset, y_start + 2 * y_step, z_up);
    wait_all_reach();
    set_site (1, x_default + x_offset, y_start + 2 * y_step, z_default);
    wait_all_reach();

    // move body
    set_site (0, x_default + x_offset, y_start + y_step, z_default);
    set_site (1, x_default - x_offset, y_start + y_step, z_default);
    set_site (2, x_default + x_offset, y_start + 2 * y_step, z_default);
    set_site (3, x_default - x_offset, y_start, z_default);
    wait_all_reach();

    // reposition leg 2
    set_site (2, x_default + x_offset, y_start + 2 * y_step, z_up);
    wait_all_reach();
    set_site (2, x_default + x_offset, y_start, z_up);
    wait_all_reach();
    set_site (2, x_default + x_offset, y_start, z_default);
    wait_all_reach();
  }
}

/*
  Spot turn to right
*/
void turn_left() {
    move_speed = spot_turn_speed;
    if (site_now[3][1] == y_start)
    {
      //leg 3&1 move
      set_site(3, x_default + x_offset, y_start, z_up);
      wait_all_reach();

      set_site(0, turn_x1 - x_offset, turn_y1, z_default);
      set_site(1, turn_x0 - x_offset, turn_y0, z_default);
      set_site(2, turn_x1 + x_offset, turn_y1, z_default);
      set_site(3, turn_x0 + x_offset, turn_y0, z_up);
      wait_all_reach();

      set_site(3, turn_x0 + x_offset, turn_y0, z_default);
      wait_all_reach();

      set_site(0, turn_x1 + x_offset, turn_y1, z_default);
      set_site(1, turn_x0 + x_offset, turn_y0, z_default);
      set_site(2, turn_x1 - x_offset, turn_y1, z_default);
      set_site(3, turn_x0 - x_offset, turn_y0, z_default);
      wait_all_reach();

      set_site(1, turn_x0 + x_offset, turn_y0, z_up);
      wait_all_reach();

      set_site(0, x_default + x_offset, y_start, z_default);
      set_site(1, x_default + x_offset, y_start, z_up);
      set_site(2, x_default - x_offset, y_start + y_step, z_default);
      set_site(3, x_default - x_offset, y_start + y_step, z_default);
      wait_all_reach();

      set_site(1, x_default + x_offset, y_start, z_default);
      wait_all_reach();
    }
    else
    {
      //leg 0&2 move
      set_site(0, x_default + x_offset, y_start, z_up);
      wait_all_reach();

      set_site(0, turn_x0 + x_offset, turn_y0, z_up);
      set_site(1, turn_x1 + x_offset, turn_y1, z_default);
      set_site(2, turn_x0 - x_offset, turn_y0, z_default);
      set_site(3, turn_x1 - x_offset, turn_y1, z_default);
      wait_all_reach();

      set_site(0, turn_x0 + x_offset, turn_y0, z_default);
      wait_all_reach();

      set_site(0, turn_x0 - x_offset, turn_y0, z_default);
      set_site(1, turn_x1 - x_offset, turn_y1, z_default);
      set_site(2, turn_x0 + x_offset, turn_y0, z_default);
      set_site(3, turn_x1 + x_offset, turn_y1, z_default);
      wait_all_reach();

      set_site(2, turn_x0 + x_offset, turn_y0, z_up);
      wait_all_reach();

      set_site(0, x_default - x_offset, y_start + y_step, z_default);
      set_site(1, x_default - x_offset, y_start + y_step, z_default);
      set_site(2, x_default + x_offset, y_start, z_up);
      set_site(3, x_default + x_offset, y_start, z_default);
      wait_all_reach();

      set_site(2, x_default + x_offset, y_start, z_default);
      wait_all_reach();
    }
}

/*
  Spot turn to right
*/
void turn_right() {
    move_speed = spot_turn_speed;

    if (site_now[2][1] == y_start)
    {
      //leg 2&0 move
      set_site(2, x_default + x_offset, y_start, z_up);
      wait_all_reach();

      set_site(0, turn_x0 - x_offset, turn_y0, z_default);
      set_site(1, turn_x1 - x_offset, turn_y1, z_default);
      set_site(2, turn_x0 + x_offset, turn_y0, z_up);
      set_site(3, turn_x1 + x_offset, turn_y1, z_default);
      wait_all_reach();

      set_site(2, turn_x0 + x_offset, turn_y0, z_default);
      wait_all_reach();

      set_site(0, turn_x0 + x_offset, turn_y0, z_default);
      set_site(1, turn_x1 + x_offset, turn_y1, z_default);
      set_site(2, turn_x0 - x_offset, turn_y0, z_default);
      set_site(3, turn_x1 - x_offset, turn_y1, z_default);
      wait_all_reach();

      set_site(0, turn_x0 + x_offset, turn_y0, z_up);
      wait_all_reach();

      set_site(0, x_default + x_offset, y_start, z_up);
      set_site(1, x_default + x_offset, y_start, z_default);
      set_site(2, x_default - x_offset, y_start + y_step, z_default);
      set_site(3, x_default - x_offset, y_start + y_step, z_default);
      wait_all_reach();

      set_site(0, x_default + x_offset, y_start, z_default);
      wait_all_reach();
    }
    else
    {
      //leg 1&3 move
      set_site(1, x_default + x_offset, y_start, z_up);
      wait_all_reach();

      set_site(0, turn_x1 + x_offset, turn_y1, z_default);
      set_site(1, turn_x0 + x_offset, turn_y0, z_up);
      set_site(2, turn_x1 - x_offset, turn_y1, z_default);
      set_site(3, turn_x0 - x_offset, turn_y0, z_default);
      wait_all_reach();

      set_site(1, turn_x0 + x_offset, turn_y0, z_default);
      wait_all_reach();

      set_site(0, turn_x1 - x_offset, turn_y1, z_default);
      set_site(1, turn_x0 - x_offset, turn_y0, z_default);
      set_site(2, turn_x1 + x_offset, turn_y1, z_default);
      set_site(3, turn_x0 + x_offset, turn_y0, z_default);
      wait_all_reach();

      set_site(3, turn_x0 + x_offset, turn_y0, z_up);
      wait_all_reach();

      set_site(0, x_default - x_offset, y_start + y_step, z_default);
      set_site(1, x_default - x_offset, y_start + y_step, z_default);
      set_site(2, x_default + x_offset, y_start, z_default);
      set_site(3, x_default + x_offset, y_start, z_up);
      wait_all_reach();

      set_site(3, x_default + x_offset, y_start, z_default);
      wait_all_reach();
    }
}

void body_right(int i) {
  set_site (0, site_now[0][0] - i, KEEP, KEEP);
  set_site (1, site_now[1][0] - i, KEEP, KEEP);
  set_site (2, site_now[2][0] + i, KEEP, KEEP);
  set_site (3, site_now[3][0] + i, KEEP, KEEP);
  wait_all_reach();
}

void body_left (int i) {
  set_site (0, site_now[0][0] + i, KEEP, KEEP);
  set_site (1, site_now[1][0] + i, KEEP, KEEP);
  set_site (2, site_now[2][0] - i, KEEP, KEEP);
  set_site (3, site_now[3][0] - i, KEEP, KEEP);
}


void hand_wave(int i) {
  float x_tmp;
  float y_tmp;
  float z_tmp;
  move_speed = 1;

  // wave leg 2 if leg 3 or 2 (i think) is in horizontal pos
  if (site_now[3][1] = y_start) {

    // body shifted 
    body_right(15);
    x_tmp = site_now [2][0];
    y_tmp = site_now [2][1];
    z_tmp = site_now [2][2];

    move_speed = body_move_speed;
    for ( int j=0; j < i; j++ ) {
      set_site (2, turn_x1, turn_y1, 50);
      wait_all_reach();
      set_site (2, turn_x0, turn_y0, 50);
      wait_all_reach();
    }

    set_site (2, x_tmp, y_tmp, z_tmp);
    wait_all_reach();
    move_speed = 2;

    // body_reshifted
    body_left (15);
  }

  // wave leg 0
  else {
    body_left(15);
    x_tmp = site_now[0][0];
    y_tmp = site_now[0][1];
    z_tmp = site_now[0][2];
    move_speed = body_move_speed;
    for (int j=0, j<i, j++) {
      set_site( 0, turn_x1, turn_y1, 50);
      wait_all_reach();
      set_site( 0, turn_x0, turn_y0, 50);
      wait_all_reach();
    }
    set_site (0, x_tmp, y_tmp, z_tmp);
    wait_all_reach();
    move_speed = 1;
    body_right(15);
  }
}

void hand_shake(int i) {
  float x_tmp;
  float y_tmp;
  float z_tmp;
  move_speed = 1;

  // shake leg 2 if leg 3 is in neutral
  if (site_now[3][1] = y_start) {
    body_right(15);
    x_tmp = site_now[2][0];
    y_tmp = site_now[2][1];
    z_tmp = site_now[2][2];

    for (int j=0; j < i; j++) {
      set_site ( 2, x_default - 30, y_start + 2 * y_step, 50 );
      wait_all_reach();
      set_site ( 2, x_default - 30, y_start + 2 * y_step, 20 );
      wait_all_reach();
    }

    set_site ( 2, x_tmp, y_tmp, z_tmp );
    wait_all_reach();
    move_speed = 1;
    body_left(15);
  }

  else {
    //leg 0 shake
    body_left(15);
    move_speed = body_move_speed;
    x_tmp = site_now[0][0];
    y_tmp = site_now[0][1];
    z_tmp = site_now[0][2];

    for ( int j=0; j < i; j++ ) {
      set_site ( 0, x_default - 30, y_start + 2 * y_step, 50 );
      wait_all_reach();
      set_site ( 0, x_default - 30, y_start + 2 * y_step, 20 );
      wait_all_reach();
    }

    set_site ( 0, x_tmp, y_tmp, z_tmp);
    wait_all_reach();
    move_speed = 1;
    body_right(15);
  }
}

void body_dance(int i) {
  float x_tmp;
  float y_tmp;
  float z_tmp;
  float body_dance_speed = 2;
  sit();
  move_speed = 1;
  set_site(0, x_default, y_default, KEEP);
  set_site(1, x_default, y_default, KEEP);
  set_site(2, x_default, y_default, KEEP);
  set_site(3, x_default, y_default, KEEP);
  wait_all_reach();
  //stand();
  set_site(0, x_default, y_default, z_default - 20);
  set_site(1, x_default, y_default, z_default - 20);
  set_site(2, x_default, y_default, z_default - 20);
  set_site(3, x_default, y_default, z_default - 20);
  wait_all_reach();
  move_speed = body_dance_speed;
  head_up(30);
  for (int j = 0; j < i; j++)
  {
    if (j > i / 4)
      move_speed = body_dance_speed * 2;
    if (j > i / 2)
      move_speed = body_dance_speed * 3;
    set_site(0, KEEP, y_default - 20, KEEP);
    set_site(1, KEEP, y_default + 20, KEEP);
    set_site(2, KEEP, y_default - 20, KEEP);
    set_site(3, KEEP, y_default + 20, KEEP);
    wait_all_reach();
    set_site(0, KEEP, y_default + 20, KEEP);
    set_site(1, KEEP, y_default - 20, KEEP);
    set_site(2, KEEP, y_default + 20, KEEP);
    set_site(3, KEEP, y_default - 20, KEEP);
    wait_all_reach();
  }
  move_speed = body_dance_speed;
  head_down(30);
}

void head_up (int i) {
  set_site (0, KEEP, KEEP, site_now[0][2] - i);
  set_site (1, KEEP, KEEP, site_now[1][2] + i);
  set_site (2, KEEP, KEEP, site_now[2][2] - i);
  set_site (3, KEEP, KEEP, site_now[3][2] + i);
  wait_all_reach();
}

void head_down (int i) {
  set_site (0, KEEP, KEEP, site_now[0][2] + i);
  set_site (1, KEEP, KEEP, site_now[1][2] - i);
  set_site (2, KEEP, KEEP, site_now[2][2] + i);
  set_site (3, KEEP, KEEP, site_now[3][2] - i);
  wait_all_reach();
}

// void head_up (int i) {
//   set_site (0, KEEP, KEEP, z_default - i);
//   set_site (1, KEEP, KEEP, z_default + i);
//   set_site (2, KEEP, KEEP, z_default - i);
//   set_site (3, KEEP, KEEP, z_default + i);
//   wait_all_reach();
// }

// void head_down (int i) {
//   set_site (0, KEEP, KEEP, z_default + i);
//   set_site (1, KEEP, KEEP, z_default - i);
//   set_site (2, KEEP, KEEP, z_default + i);
//   set_site (3, KEEP, KEEP, z_default - i);
//   wait_all_reach();
// }

void servo_service() {

  sei();      // to continue the counter in FlexiTimer2 

  static float alpha, beta, gamma;

  for (int i = 0; i < 4; i++) {
    for ( int j = 0; j < 3; j++) {
      if ( abs( site_expect[i][j] -  site_now[i][j] >= temp_speed[i][j])) {
        site_now[i][j] += temp_speed[i][j];
      }
      else 
        site_now[i][j] = site_expect[i][j];
    }

    // pass the new site_now position to get polar angles
    cartesian_to_polar (alpha, beta, gamma, site_now[i][0], site_now[i][1], site_now[i][2]);
    polar_to_servo (i , alpha, beta, gamma);
  }
  rest_counter++;
}



/* ---------------------------------------------------------------------
  - check the end position with current position
  - if not there yet, temp_speed 
*/
void set_site(int leg, float x, float y, float z) {
  float length_x = 0, length_y = 0, length_z = 0;

  // checking if the x,y,z position of the leg tip are at the site_expect yet
  // [0] -> x, [1] -> y, [2] -> z
  // site_now[leg][0] is not related with the joint but the x-coordinate of the leg tip
  if (x != KEEP) 
    length_x = x - site_now[leg][0];
  if (y != KEEP) 
    length_y = y - site_now[leg][1];
  if (z != KEEP)
    length_z = z - site_now[leg][2];
  
  float length = sqrt ( pow(length_x, 2) + pow(length_y, 2) + pow(length_z, 2) );

  /* ------------------------------------------------------------------------------------*/

  // setting the x, y, z direction speed so when it moves it will go in diagonal direction
  // set_site (1, 50, 30, 20) 
  // site_now[1][0] = 30, site_now[1][1] = 20, site_now[1][2] = 0
  // length_x = 20, length_y = 10, length_z = 20
  // length = 30
  // let move_speed = 10
  
  // x = 50, site_now[leg][0] = 30; length_x = 50 - 30 = 20
  // temp_speed[leg][0] = 20 / 30 * 10 * 1 = 6.78
  // temp_speed[leg][1] = 10 / 30 * 10 * 1 = 3.33
  // temp_speed[leg][2] = 20 / 30 * 10 * 1 = 6.78

  // site_now[leg][0] += temp_speed[leg][0] until site_now == site_expect  -->  3 ticks
  // for [leg][1] is also 3 ticks as well to move from y = 20 to 30 
  // for [leg][2] is 3 ticks away from the final position 

  // - This calculation of temp_speed finds the straight line movement for the leg tip 
  // - Dissect the straight line so that it takes equal amounts of ticks for each coordinate to get to the final position 

  /* ------------------------------------------------------------------------------------*/

  temp_speed[leg][0] = length_x/length * move_speed * speed_multiple;
  temp_speed[leg][1] = length_y/length * move_speed * speed_multiple;
  temp_speed[leg][2] = length_z/length * move_speed * speed_multiple;

  if ( x != KEEP )
    site_expect[leg][0] = x;
  if ( y != KEEP )
    site_expect[leg][1] = y;
  if ( z != KEEP )
    site_expect[leg][2] = z;

}

void cartesian_to_polar (volatile float& alpha, volatile float& beta, volatile float& gamma, volatile float x, volatile float y, volatile float z) {
  
  float v, w;
  // check if the x-coordinate is in negative position
  // if x is negative, the leg tip will be under coax ( not possible for this robot )
  w = (x >= 0 ? 1 : -1) * (sqrt (pow(x,2) + pow(y,2)));

  v = w - length_c;

  gamma = (w >= 0) ? atan2(y, x) : atan2(-y,-x);

  alpha = atan2(z, v) + acos((pow(length_a, 2)+ pow (v,2) + pow(z,2) - pow (length_b, 2)) / 2 / length_a / sqrt (pow(v,2) + pow(z,2)));

  beta = acos(( pow(length_a,2) + pow(length_b,2) - pow(v,2) - pow(z,2)) / 2/ length_a / length_b);

  // pi -> 180
  alpha = alpha / pi * 180;
  beta = beta / pi * 180;
  gamma = gamma / pi * 180;
}

// the angles get customized for each leg
void polar_to_servo (int leg, float alpha, float beta, float gamma) {
  if ( leg == 0 ) {
    alpha = 90 - alpha;
    beta = beta;
    gamma += 90;
  }

  else if ( leg == 1 ) {
    alpha += 90;
    beta = 180 - beta;
    gamma = 90 - gamma;
  }

  else if ( leg == 2 ) {
    alpha += 90;
    beta = 180 - beta;
    gamma = 90 - gamma;
  }

  else if ( leg == 3 ) {
    alpha = 90 - alpha;
    beta = beta; 
    gamma += 90;
  }

  servo[leg][0].write(alpha);
  servo[leg][1].write(beta);
  servo[leg][2].write(gamma);
}

void wait_all_reach() {
  for (int leg = 0; leg < 4; leg++ ) {
    wait_reach(leg);
  }
}

void wait_reach(int leg) {
  while (1) {
    if (site_now[leg][0] == site_expect[leg][0])
      if (site_now[leg][1] == site_expect[leg][1]) 
        if (site_now[leg][2] == site_expect[leg][2])
          break;
  }
}


