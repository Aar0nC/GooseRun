
const int FALSE = 0; 
const int TRUE = 1; 

typedef struct{
   int x, y, width, height; 
} rect; 

rect goose;  
rect obstacles[NUM_OBSTACLES]; 

const int NUM_OBSTACLES = 12; 
const int NUM_BUMPS =  16; 

int yInitialVelocity = 0 //this is plugged into the kinematic equation
const int INITIAL_JUMP_VELOCITY = 10; 
const int  INITIAL_ITERS_FOR_UPDATE = 5; / 

int timeSinceJump = 0 
int score = 0 
int itersForUpdate = INITIAL_ITERS_FOR_UPDATE; 
int loopCount = 0; 
int isPlayerDead = FALSE; 

void checkCollision() {
  for (int i=0; i < NUM_OBSTACLES; i++) {
  if ((((goose.x >= obstacles[i].x - 0.5*obstacles[i].width) && (goose.x <= obstacles[i].x + 0.5*obstacles[i].width)))) && 
          (((goose.y >= obstacles[i].y - 0.5*obstacles[i].height) && (goose.y <= obstacles[i].y + 0.5*obstacles[i].height))) {
          isPlayerDead = TRUE;
        }
  }
}

loop {
  checkInputs(); 
  if(loopCount == itersForUpdate){
     updateMap(); 
     checkCollision(); 
     updateScore(); 
     loopCount = -1; 
  }  
  loopCount++;
}

void  checkInputs(){
   
  long lBtn1; 
  // the number of the pushbutton pin
  lBtn1 = GPIOPinRead(BTN1Port, BTN1);

  if(lBtn1 == BTN1 && yInitialVelocity == 0) {
    yInitialVelocity = INITIAL_JUMP_VELOCITY
  }
}
