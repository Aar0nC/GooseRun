extern "C" {
#include <delay.h>
#include <FillPat.h>
#include <I2CEEPROM.h>
#include <LaunchPad.h>
#include <OrbitBoosterPackDefs.h>
#include <OrbitOled.h>
#include <OrbitOledChar.h>
#include <OrbitOledGrph.h>
}

#define DEMO_0    0
#define DEMO_1    2
#define DEMO_2    1
#define DEMO_3    3
#define RED_LED   GPIO_PIN_1
#define BLUE_LED  GPIO_PIN_2
#define GREEN_LED GPIO_PIN_3
#define NUM_OBSTACLES 12
#define NUM_BUMPS 16
#define FALSE 0 
#define TRUE 1
#define GRAVITY 10
#define INITIAL_ITERS_FOR_UPDATE 10000
#define START_NUM_LIVES 3
#define GOOSE_WIDTH 7
#define GOOSE_HEIGHT 10
#define GOOSE_HEIGHT_BMP 16
#define DEFAULT_GOOSE_X 4
#define WAIT_TIME_BETWEEN_COLLISIONS 100000
int timeSinceLastCollision = 0;


#define MAP_HEIGHT 32
#define MAP_WIDTH 128

// Aman macros for flying
#define ENERGY_LOSE_DELAY 5
#define ENERGY_GAIN_DELAY 20
int isFlying = FALSE;

//Ethan macro stuff for jumping
//(timeSinceJump * 2.6 - 0.12 * timeSinceJump * timeSinceJump);
#define INITIAL_JUMP_VELOCITY 2.9
#define GRAVITY_MULTIPLIER 0.12

//SPENCER MACRO
#define OBSTACLE_SPACING  30
#define OBSTACLE_MIN_WIDTH 3
#define OBSTACLE_MAX_WIDTH 7
#define OBSTACLE_MAX_HEIGHT 6
#define OBSTACLE_MIN_HEIGHT 3

//#define OBSTACLE_GO_TO_X_LOC_RANDOM_SPACING 700
#define OBSTACLE_GO_TO_X_LOC 850
int obstacleSpacing = 700;

//Scott macro stuff
#define BUMP_SPACING 10//all used by createBumps() function
#define BUMP_MIN_WIDTH 1
#define BUMP_MAX_WIDTH 3
#define GROUND_HEIGHT 1
#define BUMP_HEIGHT 1
#define BUMP_START_X_LOC 10
#define TOP_BUMP_Y MAP_HEIGHT-(GROUND_HEIGHT+BUMP_HEIGHT)
#define BUMP_DEFAULT_GO_TO_X 133;
int bumpsCreated = FALSE;

typedef struct{
   int x, y, width, height; 
} rect; 

rect goose;  
rect obstacles[NUM_OBSTACLES];
rect bumps[NUM_BUMPS];


int numLives = START_NUM_LIVES;
int yInitialVelocity = 0; //this is plugged into the kinematic equation
 

int timeSinceJump = 0;
int score = 0;
int itersForUpdate = INITIAL_ITERS_FOR_UPDATE; 
int loopCount = 0; 
int isPlayerDead = FALSE;
int isPlayerInAir = FALSE;

// Flying energy int
int flyingEnergy = 100;

void DeviceInit();
void updateMap();
void checkInputs();
void checkCollision(); 
void updateScore(); 
void createBumps();
void setObstacles();

/*char gooseSprite[] = {
  0x40, 0x60, 0xF0, 0x70, 0xFF, 0x73, 0x02, 0x80, 0xFE, 0x86, 0x04,
  0x04, 0x0E, 0x0E, 0x0F, 0x0F, 0xFF, 0x8F, 0x0F, 0xFF, 0x8F, 0x07
};*/
char gooseSprite[] = {
  0x40, 0x60, 0xF0, 0x70, 0xFF, 0x73, 0x02,
  0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00,
};

char  chSwtCur;
char  chSwtPrev;
bool  fClearOled;

void setup()
{
  // put your setup code here, to run once:
  DeviceInit();
  createBumps();
  createObstacles();
  goose.x = DEFAULT_GOOSE_X;
  goose.height = GOOSE_HEIGHT;
  goose.width = GOOSE_WIDTH;
  itersForUpdate = INITIAL_ITERS_FOR_UPDATE;
  //Serial.begin(9600);
  randomSeed(analogRead(A0));
}

void DeviceInit()
{
  /*
   * First, Set Up the Clock.
   * Main OSC     -> SYSCTL_OSC_MAIN
   * Runs off 16MHz clock -> SYSCTL_XTAL_16MHZ
   * Use PLL      -> SYSCTL_USE_PLL
   * Divide by 4    -> SYSCTL_SYSDIV_4
   */
  SysCtlClockSet(SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_USE_PLL | SYSCTL_SYSDIV_4);

  /*
   * Enable and Power On All GPIO Ports
   */
  //SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOA | SYSCTL_PERIPH_GPIOB | SYSCTL_PERIPH_GPIOC |
  //            SYSCTL_PERIPH_GPIOD | SYSCTL_PERIPH_GPIOE | SYSCTL_PERIPH_GPIOF);

  SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOA );
  SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOB );
  SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOC );
  SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOD );
  SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOE );
  SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOF );
  /*
   * Pad Configure.. Setting as per the Button Pullups on
   * the Launch pad (active low).. changing to pulldowns for Orbit
   */
  GPIOPadConfigSet(SWTPort, SWT1 | SWT2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);

  GPIOPadConfigSet(BTN1Port, BTN1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
  GPIOPadConfigSet(BTN2Port, BTN2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);

  GPIOPadConfigSet(LED1Port, LED1, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);
  GPIOPadConfigSet(LED2Port, LED2, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);
  GPIOPadConfigSet(LED3Port, LED3, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);
  GPIOPadConfigSet(LED4Port, LED4, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);

  /*
   * Initialize Switches as Input
   */
  GPIOPinTypeGPIOInput(SWTPort, SWT1 | SWT2);

  /*
   * Initialize Buttons as Input
   */
  GPIOPinTypeGPIOInput(BTN1Port, BTN1);
  GPIOPinTypeGPIOInput(BTN2Port, BTN2);

  /*
   * Initialize LEDs as Output
   */
  GPIOPinTypeGPIOOutput(LED1Port, LED1);
  GPIOPinTypeGPIOOutput(LED2Port, LED2);
  GPIOPinTypeGPIOOutput(LED3Port, LED3);
  GPIOPinTypeGPIOOutput(LED4Port, LED4);

  /*
   * Enable ADC Periph
   */
  SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

  GPIOPinTypeADC(AINPort, AIN);

  /*
   * Enable ADC with this Sequence
   * 1. ADCSequenceConfigure()
   * 2. ADCSequenceStepConfigure()
   * 3. ADCSequenceEnable()
   * 4. ADCProcessorTrigger();
   * 5. Wait for sample sequence ADCIntStatus();
   * 6. Read From ADC
   */
  ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
  ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH0);
  ADCSequenceEnable(ADC0_BASE, 0);

  /*
   * Initialize the OLED
   */
  OrbitOledInit();

  /*
   * Reset flags
   */
  chSwtCur = 0;
  chSwtPrev = 0;
  fClearOled = true;

}
//////makes the obstacles -Spencer
void createObstacles(){
   int i; 
   
   for (i = 0; i < NUM_OBSTACLES; i++){
       rect obstacle; 
       obstacle.width = random(OBSTACLE_MIN_WIDTH, OBSTACLE_MAX_WIDTH);
       obstacle.x = OBSTACLE_GO_TO_X_LOC + random(-obstacleSpacing, obstacleSpacing );
       obstacle.height = random(OBSTACLE_MIN_HEIGHT,OBSTACLE_MAX_HEIGHT);
       obstacle.y = MAP_HEIGHT - (GROUND_HEIGHT + obstacle.height);
       obstacles[i] = obstacle;
   }
}

///////update map and make bumps ---> Scott
void createBumps(){  
   int i;
   //random(x, y), inclusive x, exclusive y
   for (i = 0; i < NUM_BUMPS; i++){
       rect bump; 
       bump.x = BUMP_START_X_LOC + i * BUMP_SPACING;
       bump.y = TOP_BUMP_Y;
       bump.height = BUMP_HEIGHT;
       bump.width = random(BUMP_MIN_WIDTH, BUMP_MAX_WIDTH + 1);
       bumps[i] = bump;
   }
}

void updateMap(){
  //128 by 32
 // if (bumpsCreated == 0){//make all the bumps, only once
 //     createBumps();
 //     bumpsCreated = 1;     
 // }
  
  //move each bump each time, make sure to wrap
  
  int i;
  //move bumps to the left
  for (i = 0; i < NUM_BUMPS; i++){
      bumps[i].x--;
      if (bumps[i].x + bumps[i].width <= 0){//at end left of screen
          bumps[i].x = BUMP_DEFAULT_GO_TO_X;//set to to far right 
      }  
  }

  //move obstacles to the left
  for (i = 0; i < NUM_OBSTACLES; i++){
    obstacles[i].x--;
    if (obstacles[i].x + obstacles[i].width <= 0){
        obstacles[i].x = OBSTACLE_GO_TO_X_LOC + random(-obstacleSpacing,obstacleSpacing );
    }
  }


 
  goose.y = MAP_HEIGHT - GROUND_HEIGHT - goose.height - (timeSinceJump * INITIAL_JUMP_VELOCITY - GRAVITY_MULTIPLIER* timeSinceJump * timeSinceJump);
  if (goose.y + goose.height > MAP_HEIGHT - GROUND_HEIGHT) {
    goose.y = MAP_HEIGHT - GROUND_HEIGHT - goose.height;
    timeSinceJump = 0;
    yInitialVelocity = 0;
    isPlayerInAir = FALSE;
  }
  
  if (isPlayerInAir == TRUE) {
    if (button1 != BTN1 || button2 != BTN2)
      timeSinceJump++; 
    else
      flyingEnergy -= ENERGY_LOSE_DELAY;
  }
}

void updateScreen() { //redraws the screen based on the new map 
  if (numLives == 0){
      OrbitOledClearBuffer();
      OrbitOledSetCursor(0, 0);
      OrbitOledPutString("  You suck at    this game you     big dumbo"); //i wish i could come up with chirps this good  
      OrbitOledUpdate();
      return;
  }
  
  OrbitOledClearBuffer();
  OrbitOledMoveTo(0, MAP_HEIGHT - GROUND_HEIGHT);
  OrbitOledFillRect(MAP_WIDTH, MAP_HEIGHT);
  
  OrbitOledSetFillPattern(OrbitOledGetStdPattern(1));
  for (int i = 0; i < NUM_OBSTACLES; i++) { //draw the obstacles
     OrbitOledMoveTo(obstacles[i].x, obstacles[i].y);
     OrbitOledFillRect(obstacles[i].x + obstacles[i].width, obstacles[i].y + obstacles[i].height);
  }
  for (int i = 0; i < NUM_BUMPS; i++) { //draw the bumps
     OrbitOledMoveTo(bumps[i].x, bumps[i].y);
     OrbitOledFillRect(bumps[i].x + bumps[i].width, bumps[i].y + bumps[i].height);
  }
   OrbitOledMoveTo(goose.x, goose.y); //draw the goose
   OrbitOledPutBmp(GOOSE_WIDTH, GOOSE_HEIGHT_BMP, gooseSprite);
   
   OrbitOledSetCursor(15, 0);
   if (score == 0) OrbitOledPutChar('0');
   else {
     int scoreHold = score, cursorCount = 15;
     while (scoreHold > 0) {
       OrbitOledPutChar(scoreHold % 10 + 48);
       scoreHold /= 10;
       OrbitOledSetCursor(cursorCount--, 0);
     }
   }
   
   OrbitOledUpdate();
 }

void checkCollision() {
  if (timeSinceLastCollision >= WAIT_TIME_BETWEEN_COLLISIONS){
    timeSinceLastCollision = 0;
        for (int i=0; i < NUM_OBSTACLES; i++) {
            if (obstacles[i].x < MAP_WIDTH    &&    obstacles[i].x + obstacles[i].width <= DEFAULT_GOOSE_X + GOOSE_WIDTH  ){
              if (obstacles[i].y + obstacles[i].height <= goose.y + GOOSE_HEIGHT){
                if (numLives >0){
                    numLives--;
                }
         
        //isPlayerDead = FALSE;//should be true. false for obstacle making testing
              }
            }
  
            /*if (((DEFAULT_GOOSE_X >= obstacles[i].x - 0.5*obstacles[i].width) && (goose.x <= obstacles[i].x + 0.5*obstacles[i].width)) && 
               ((goose.y >= obstacles[i].y - 0.5*obstacles[i].height) && (goose.y <= obstacles[i].y + 0.5*obstacles[i].height))) {
                 isPlayerDead = TRUE; //should be TRUE
            }*/
          }
  }

}


void loop() {

  checkInputs(); 
  if(loopCount == itersForUpdate){//10000
     updateMap(); 
     checkCollision(); 
     updateScreen(); 
     loopCount = -1;
     score++;
  }  
  loopCount++;
  timeSinceLastCollision++;
}

void checkInputs(){
   
  long button1; 
  long button2;
  // the number of the pushbutton pin
  button1 = GPIOPinRead(BTN1Port, BTN1);
  button2 = GPIOPinRead(BTN2Port, BTN2);

  if ((button1 == BTN1 || button2 == BTN2) && isPlayerInAir == FALSE) {
      yInitialVelocity = INITIAL_JUMP_VELOCITY;
      timeSinceJump = 0;
      isPlayerInAir = TRUE;
      isFlying = TRUE;
  }
  
  if ((button1 != BTN1 || button2 != BTN2) && isPlayerInAir == TRUE)
    isFlying = FALSE;
  
}


