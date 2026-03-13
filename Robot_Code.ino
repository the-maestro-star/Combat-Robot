#include <PS4Controller.h> //PS4 controller library that will allow us to communicate with the controller and read its outputs
#include <ESP32Servo.h> //Library that allows us to control servo motors, brushed motors, and brushless motors using PWM signals 
//#include <WiFi.h>
#include <math.h>

//Declare objects for each motor. BDC stands for brushed DC and BLDC stands for brushless DC
Servo LeftServo; //Left Servo
Servo RightServo; // Right Servo
Servo BDC1_forward;//left drive motor forward
Servo BDC1_backward; //left drive motor backward
Servo BDC2_forward;//right drive motor forward
Servo BDC2_backward;//right drive motor backward
Servo BLDC1;
//Initialize our variables (variables will be explained as they are used)
float value;
float xvalue;
float yvalue;
float right_stick_x;
float xvalueDelay = 0;
float yvalueDelay = 0;
float rumble = 0;
float leftRumble = 0;
float rightRumble = 0;

// Function to find Position Angle
    float find_angle(float x, float y){
      float angle = atan2(y,x);
      angle = angle * 180.0/PI; // convert angle to degrees
      return angle;
    }
// Function to drive drive motors
  void drive(float x, float y) {
    float speed = sqrt(x*x + y*y) * (255.0 / 90.0);
    if(y > 0){
        BDC1_forward.write(speed);
        BDC2_forward.write(speed);
    } else if(y < 0){
        BDC1_backward.write(speed);
        BDC2_backward.write(speed);
    } else {
        if(x < 0){
            BDC1_backward.write(speed);
            BDC2_backward.write(speed);
        } else if(x > 0){
            BDC1_forward.write(speed);
            BDC2_forward.write(speed);
        }
    }
}


//In the setup function is code that only runs once at the start of the program (when your robot is turned on)
void setup() {
  //Print the esp-32's MAC address (you probably won't need this)
  Serial.begin(115200);
  //Serial.println();
  //Serial.print("ESP Board MAC Address:  ");
  //Serial.println(WiFi.macAddress());

  //Uncomment the code for the controller you want to pair to below. If you're using your own controller that isn't listed here, write another line and put its MAC address within PS4.begin();

  //Blue Genuine Controller
  //PS4.begin("78:21:84:7f:ee:40");
  //Black Knockoff Controller
  //PS4.begin("e8:d8:19:51:01:b2");
  //Black Genuine Controller
  PS4.begin("30:C6:F7:29:75:08");
  //White Knockoff Controller
  //PS4.begin("e8:d8:19:51:01:b3");

  //Assign motor signal wires to their pins on the esp-32
  LeftServo.attach(7); //PWM Signal output from pin 7
  RightServo.attach(8);//PWM Signal output from pin 8
  BDC1_forward.attach(2); // PWM signal output from pin 2.
  BDC1_backward.attach(3); // PWM signal output from pin 3.
  BDC2_forward.attach(4); // PWM signal output from pin 4.
  BDC2_backward.attach(5); // PWM signal output from pin 5.
  BLDC1.attach(6); // PWM signal output from pin 6.

  PS4.attach(onEvent);

  PS4.setLed(185, 9, 19); //Set the controller's light bar color! Uses r,g,b
  PS4.sendToController(); //Send data to controller (used for setting light bar color and setting rumble)

  Serial.println("Ready.");
}

void loop() {
  if (PS4.isConnected()) {
    // R2 Weapon Control
    if(PS4.R2Value()>10){
      value = map(PS4.R2Value(), 0, 255, 30, 90);
      rightRumble = PS4.R2Value();
    }else if(PS4.L2Value()>10){
      
      leftRumble = PS4.L2Value();
    }else{
      leftRumble = rightRumble = 0;
    }
    BLDC1.write(value);
    PS4.setRumble(leftRumble, rightRumble);
    PS4.sendToController();

  //Using only left stick to drive
  //Take values from joysticks (from -128 to 127) and map them to a more usable range (-90 to 90)
    xvalue = map(PS4.LStickX(), -128, 127, -90, 90);
    yvalue = map(PS4.LStickY(), -128, 127, 90, -90);

 
  //Declaring the servo angle
  float angle;
    if (yvalue >= 0) {
    angle = find_angle(xvalue, yvalue);
    } else {
    angle = find_angle(-xvalue, -yvalue);
    }
    //Move Servos to position
    angle = constrain(angle, 0, 180);
    LeftServo.write(angle);
    RightServo.write(angle);
    

    delay(10); //delay before moving the wheels

    //Moving the motors
    if(abs(xvalue) < 10) xvalue = 0; //Creating x deadzone 
    if(abs(yvalue)<10) yvalue = 0; //Creating y deadzone 
    float old_right_stick_x = right_stick_x; //This is to make sure that when the left stick is moving it effectively ignores input from right stick because of interference
    right_stick_x = 0;
    drive(xvalue,yvalue); //Drive robot
    delay(5);
    right_stick_x = old_right_stick_x;

    //Rotation of Bot Using Right Stick
    
    right_stick_x = map(PS4.RStickX(), -128, 127, -90, 90);
    if(abs(right_stick_x) < 10) right_stick_x = 0; //Creating x deadzone 
    float new_speed = map(abs(right_stick_x),0,90,0,120);// Chose 0 to 120 because I do not want it to rotate too fast
    float oldxvalue = xvalue;
    float oldyvalue = yvalue;
    xvalue = 0; //This is to make sure that when the right stick is moving it effectively ignores input from left stick because of interference
    yvalue = 0;//This is to make sure that when the right stick is moving it effectively ignores input from left stick because of interference
    if (right_stick_x < 0) BDC2_forward.write(new_speed);
    if (right_stick_x > 0) BDC1_forward.write(new_speed);
    delay(5);
    xvalue = oldxvalue;
    yvalue = oldyvalue;
    
  
  //Print out motor values (used for debugging, you probably wont need this)
  Serial.print("Left Stick x value = ");
  Serial.println(xvalue);
  Serial.print("Left_Stick y value = ");
  Serial.println(yvalue);
  Serial.print("Angle = ");
  Serial.println(angle);
  Serial.print("Speed = ");
  Serial.println(speed);
   }
}
