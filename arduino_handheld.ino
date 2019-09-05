/*led control library from github.com/wayoda/ledcontrol */
#include <LedControl.h>

//used pins
char din = 12;
char cs = 11;
char clk = 10;


char soundPin = 13;

char leftButton= 7;
char rightButton= 8;


LedControl lc=LedControl(din,clk,cs,1);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lc.shutdown(0,false); 
  lc.setIntensity(0,5); 
  lc.clearDisplay(0);
}

  ///////////
  //classes//
  ///////////

class Controller{
  private:
  bool leftButton;
  bool rightButton;
  public:
  Controller(short lButton, short rButton){
    pinMode(lButton, INPUT_PULLUP);
    pinMode(rButton, INPUT_PULLUP);
    leftButton = digitalRead(lButton);
    rightButton = digitalRead(rButton);
  }

};

class Sound{
  private:
  short soundPin;
  
  public:
  //used to dynamically turn on and off the beeper
  Sound(short soundpin){
    pinMode(13, OUTPUT);
    soundPin = soundpin;
    off();
  }

  void on(){
    digitalWrite(soundPin, HIGH);
  }

  void off(){
    digitalWrite(soundPin, LOW);
  }
};
class Screen{
  private:
  short data;
  short chipSelect;
  short clockPin;
  public:
  short width;
  short height;
  byte field[8];
  Screen(char din, char cs, char clk){
    data = din;
    chipSelect = cs;
    clockPin = clk;
    width = 8;
    height = 8;
    LedControl lc=LedControl(din,cs,clk,1);
    lc.shutdown(0, false);
    lc.setIntensity(0,8);
    for(int index=0;index<lc.getDeviceCount();index++) {
        lc.shutdown(index,false); 
    }
    reset();
  }

  void reset(){
    for(char i=0; i<width; i++){
      field[i]= B00000000;
    }
  }
  
  void draw(){
    
    for(char i=0; i<width; i++){
      lc.setRow(0,i,field[i]);
    }
    reset();
  }
};

class Game{
  private:
  short fps = 16;
  public:
  Controller controller;
  Sound sound;
  Screen screen;
  Game(Controller cntrl, Sound snd, Screen scrn)
  :controller(cntrl), sound(snd), screen(scrn){
    byte* field = screen.field;
  }

  void draw(){
     //prints a new frame
    for(byte r = 0; r < screen.height; r++){
        Serial.print(screen.field[r]);
      Serial.print("\n");
    }
  }
};

////////
//pong//
////////

  //ball//
class Ball{

  public:
  short x;
  short y;
  short x_speed;
  short y_speed;
  Sound sound;
  public:
  Ball(short field_width, Sound so)
  :sound(so){
    x = field_width/2;
    y = 0;
    x_speed = 1;
    y_speed = 1;
  }

  //function that is to be executed every second to update the ball
  short update(short field_width, short field_height, short paddle_x, short paddle_width, Sound sound)
  {
    x=x+x_speed;
    y=y+y_speed;

    //checks if the ball has hit any vertical walls
    if(x==0 || x==field_width-1){
      x_speed=x_speed*-1;
    }

    //checks if the ball has hit the paddle or any horizontal walls 
    else if(y==0 || y==field_height-2 && x>=paddle_x && x<paddle_width+paddle_x){
      y_speed=y_speed*-1;
    }

    if(y>field_height-2){
      x = field_width/2;
      y = 0;
      Serial.println("too bad");
    }
    if(x==0 || x==field_width-1 || y==0 || y==field_height-2 && x>=paddle_x && x<paddle_width+paddle_x || y>field_height-2){
      sound.on();
    }else{
      sound.off();
    }
  }
};

  //paddle//
class Paddle{
  public:
  short width;
  short x;
  short y;
  Paddle(short paddle_width, short field_width, short field_height){
    width=paddle_width;
    x=(field_width)/2-width/2;
    y=field_height-1;
  }

  //function that is to be executed every second to update the paddle
  void update(){
    if(digitalRead(leftButton)==1 && x > 0){
      x--;
    }

    if(digitalRead(rightButton)==1 && x+width < 8){
      x++;
    }
  }
};

class Pong: public Game{
  public:
  Paddle paddle;
  Ball ball;
  Pong(Controller controller, Sound sound, Screen screen, Ball bo, Paddle po)
  : Game(controller, sound, screen), paddle(po), ball(bo)
  {
  
    
  }

  //handles the updates
  void update(){
    paddle.update();
    ball.update(screen.width, screen.height, paddle.x, paddle.width, sound);
  }

  //creates a new frame
  void create_frame(){
    screen.reset();
    screen.field[ball.y] = B10000000>>ball.x;
    screen.field[7] = B11100000>>paddle.x;
    screen.draw();
  }
};

///////////
//objects//
///////////
Controller controller = Controller(12, 8);
Sound sound = Sound(soundPin);
Screen screen = Screen(12, 10, 11);
Ball ball = Ball(8, sound);
Paddle paddle = Paddle(3, 8, 8);
Sound beeper = Sound(soundPin);
Pong game = Pong(controller, sound, screen, ball, paddle);

void loop() {
  // allows the game to be played in the serial prompt by erasing all the older frames, sort of.
  for(int i=0; i<20; i++){
     Serial.print("\n");
  }

  
  //updates the positions of the entities
  game.update();
  game.create_frame();
  
  
  delay(500);
}
