
const FALSE = 0; 
const TRUE = 1; 

typedef struct{
   int x, y, width, height; 
} rect; 

rect goose;  
rect obstacles[NUM_OBSTACLES]; 

const int NUM_OBSTACLES = 12; 
const in NUM_BUMPS  16; 

int yInitialVelocity = 0 //this is plugged into the kinematic equation
const int INITIAL_JUMP_VELOCITY = 10; 
const  INITIAL_ITERS_FOR_UPDATE = 5; / 

int timeSinceJump = 0 
int score = 0 
int itersForUpdate = INITIAL_ITERS_FOR_UPDATE; 
int loopCount = 0; 
int isPlayerDead = FALSE; 

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
