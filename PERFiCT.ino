#include <avr/EEPROM.h> //TINAH mem
#include <phys253.h> //ask Jon
#include <LiquidCrystal.h> //LCD funcs

#define N 0
#define E 1
#define S 2
#define W 3
#define STRAIGHT 0
#define RIGHT 1
#define BACK 2
#define LEFT -1
#define MAX_MOTOR_SPEED 255
#define BUZZER_PIN 1
#define LEFT_MOTOR 0
#define RIGHT_MOTOR 2
#define GM7 3
#define GARBAGE -100

/*
  Function Prototypes by File
*/
// Main
void TapeFollow(void);
void PrintToLCD(void);
void enableExternalInterrupt(unsigned int, unsigned int);
// PassengerPickup
int PickupPassenger(int);
int CheckForPassenger(void);
void DropoffPassenger(int);
// Intersection
void AreWeThereYet(void);
void amILost(void);
void ProcessIntersection(void);
void checkToSeeIfWeKnowWhereWeAre(void);
bool sortaEqual(int, int);
// Decisions
void TurnDecision(void);
// Collisions
void CollisionCheck(void);
void TurnAround(int, int, volatile unsigned int&, volatile unsigned int&);
void TurnCW(void);
void TurnCCW(void);
void ReverseLeft(void);
void ReverseRight(void);
// Menu Functions
void MainMenu(void);
void Menu(void);
void ViewDigital(void);
void ViewAnalog(void);
void ControlArm(void);
void altMotor(void);
void PickupPassengerMain(void);
void jettPace(void);

void (*menuFunctions[])() = {Menu, ViewDigital, ViewAnalog, ControlArm, PickupPassengerMain, altMotor, jettPace};
int countMainMenu = 7;
const char *mainMenuNames[] = {"Change Vars", "View Digital In", "View Analog In", "Control Arm", "Pickup Passenger", "Alt Motor", "Jett Pace"};

/* Store a variable in TINAH mem*/
class MenuItem
{
  public:
    String    Name;
    uint16_t  Value;
    uint16_t* EEPROMAddress;
    static uint16_t MenuItemCount;
    MenuItem(String name)
    {
      MenuItemCount++;
      EEPROMAddress = (uint16_t*)(2 * MenuItemCount);
      Name      = name;
      Value         = eeprom_read_word(EEPROMAddress);
    }
    void Save()
    {
      eeprom_write_word(EEPROMAddress, Value);
    }
};



uint16_t MenuItem::MenuItemCount = 0;
/* Add the menu items here */
MenuItem Gain             = MenuItem("Total Gain");
MenuItem Speed            = MenuItem("Speed");
MenuItem ProportionalGain = MenuItem("P-gain");
MenuItem DerivativeGain   = MenuItem("D-gain");
MenuItem IntersectionGain = MenuItem("Int-Gain");
MenuItem menuItems[]      = {Gain, ProportionalGain, DerivativeGain, Speed, IntersectionGain};
int divisors[] = {8, 8, 8, 1, 2}; //divides gains and speeds by this number


/*
  Define ALL Pins
*/
// Digital:
// Tape follwing QRDs
/*q0:far left, q1:left centre, q2right centre, q3: far right*/
#define q0 5
#define q1 6
#define q2 4 
#define q3 7
int qrdVals[4];

//Switches


// These are indices for array
#define FRONT_BUMPER 0 //change to _INDEX
#define FRONT_RIGHT_BUMPER 1
#define RIGHT_BUMPER 2
#define REAR_BUMPER 3
#define LEFT_BUMPER 4
#define FRONT_LEFT_BUMPER 5

// Constants for pin on TINAH
#define OR 0
#define FRONT_BUMPER_PIN 9
#define FRONT_RIGHT_BUMPER_PIN 10
#define RIGHT_BUMPER_PIN 13
#define REAR_BUMPER_PIN 12
#define LEFT_BUMPER_PIN 11
#define FRONT_LEFT_BUMPER_PIN 8
int switchVals[6] = {0};

// Analog
//IR
#define ArmIRpin 1
#define leftIR 4  
int leftIRVal = -1;   int leftIRValMax = -1;
#define rightIR 5  
int rightIRVal = -1;  int rightIRValMax = -1;

#define topIRBack 2
#define topIRLeft 3
#define topIRRight 0

/*
  GLOBAL VARIABLES
*/
int numOfIttrs = 0;

// Tape vollowing variables
int error = 0;
int kp;
int kd;
int g;
int pastError = 0;
int recError;
int prevError;
int q = 0;
int m = 0;
int vel;
int p;
int d;
int correction;
int statusCountTapeFollow = 0;
int tapeFollowVel;
int avgCorrection = 0;

//Interrupt Counts:
volatile unsigned int leftCount = 0;
volatile unsigned int rightCount = 0;
unsigned int collisionDetected = 0;
unsigned int collisionCount = 0;

//NAV VARIABLES -- decisions
int topIR0, ir0 = 0;
int topIR1, ir1 = 1;
int topIR2, ir2 = 2;
int directionOfDropZone; // 0 to 359 degrees (bearings).
int topIRSensitivity = 200;
int offset;
int currentNode;
int robotDirection;
int discrepancyInLocation = false;
int accuracyInIR = 60;
int tempInt;
int dir;
int nextTempNode;
int desiredDirection;
int highestProfit;
int profits[4] = {0};
int lastIntersectionType;

int initialProfitMatrix[4][20];
int profitMatrix[4][20];

//edge matrix stuff
int theMap[4][20] = { // theMap[currentInd][dir] = [toIndex]
  // 0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19
  { -1, -1, -1, -1, -1, -1,  1,  2,  3, -1,  0,  6,  7,  7,  8,  4, 10, 11, 14, 15}, //N
  { -1, -1, -1, -1, -1,  6, -1, 13,  9, -1, 11, 12, -1, 14, 15, -1, 17, 18, 19, -1}, //E
  { 10,  6,  7,  8, 15, -1, 11, -1, 14, -1, 16, 17, 13, 12, 18, 19, -1, -1, -1, -1}, //S
  { -1, -1, -1, -1, -1, -1,  5, 12, -1,  8, -1, 10, 11, -1, 13, 14, -1, 16, 17, 18} //W
}; //dont change this
int rotEncoder[4][20] = {
	//?? Idk if we need this but it'd be in the same format as theMap.
};
//                             0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19
int dirToDropoff[20]        = {S, S, S, S, S, E, S, E, S, W, S, S, W, E, S, S, E, E, W, W}; // Direction of dropoff zone from each intersection
int secondDirToDropoff[20]  = {S, S, S, S, S, E, N, W, N, W, E, W, S, S, E, W, N, N, N, N};
int bearingToDropoff[20] = {120, 160, 180, 200, 240, 120, 150, 180, 210, 240, 110, 120, 160, 200, 240, 250, 100, 100, 260, 260}; // gives bearing to dropoff from each node
int distToDropoff[20] = {4, 4, 5, 4, 4, 4, 3, 4, 3, 4, 3, 2, 3, 3, 2, 3, 2, 1, 1, 2};
int stuckLikelyhood[20] = {8, 8, 8, 8, 8, 8, 7, 4, 7, 8, 5, 1, 4, 4, 1, 5, 4, 2, 2, 4};
int numOfDolls[20] = {2, 3, 2, 3, 2, 3, 8, 6, 8, 3, 6, 7, 5, 5, 7, 6, 3, 7, 7, 3};
int intersectionType[20]; // stores type of each intersection ie. 4-way, 4 bit boolean {NSEW} T/F values

int currentEdge[2];
int currentDir;
int dirAfterInt;
int possibleTurns[3] = {0}; // left, straight, right True/False values - necessary??
int desiredTurn = GARBAGE;
int turnActual = GARBAGE;
int nodeMat[20][20] = {0}; //nodeMat[fromIndex][toIndex] = dir
int countInIntersection = 0;
#define maxInIntersection 1300
int countTurning = 0;
#define maxTurning 3000
int leftTurnPossible = 0;
int rightTurnPossible = 0;
int intGain;
int lostCount = 0;
int qrdToCheck;
int loopNum = 1;
int statusCount = 0;
int statusLast = 0;
#define pathConfidence 20
int loopsSinceLastInt = 0;
int leavingCount = 0;
int tapeFollowCountInInt = 0;
int noStraightCount = 0;

// checkToSeeIfWeKnowWhereWeAre variables
int rightEncoderAtLastInt = 0;
int leftEncoderAtLastInt = 0;
int rightDiff;
int leftDiff;
int diff;
#define curveInsideCount 300
#define curveOutsideCount 350
#define straightCount 450

int count180 = 0;
int statusCount180 = 0;
int countLeft180 = 0;
int countRight180 = 0;

// Loop timing variables
unsigned long t1 = 0;
unsigned long t2 = 0;
int loopTime;
unsigned long startTime;

// Passenger Pickup
#define sideIRMin 650
#define armIRMin 250
#define countToDropoff 350 // TODO Change
#define dropWidth  80
#define PICKUPSUCCESSTHRESH 400
int failedPickup = 0;
int passengerSide;
int passengerSeenCount = 0;
int stopTime1 = 0;
int stopTime2 = 0; 
int leftInitial = GARBAGE;
int rightInitial = GARBAGE;


// Angles of straight arm and open claw
#define armHome 80
#define clawOpen 160
#define clawMid 80
#define clawClose 10

//int desiredTurns[] = {STRAIGHT, LEFT, LEFT, RIGHT, LEFT, STRAIGHT, LEFT, STRAIGHT, RIGHT, RIGHT, STRAIGHT, STRAIGHT, RIGHT, STRAIGHT, BACK}; //these are temporary and only for testing
int desiredTurns[] = {STRAIGHT, LEFT, STRAIGHT, LEFT, LEFT, LEFT, STRAIGHT, STRAIGHT, STRAIGHT, LEFT, STRAIGHT, RIGHT};
//int desiredTurns[] = {STRAIGHT, LEFT, LEFT, RIGHT, LEFT, STRAIGHT, STRAIGHT, LEFT, STRAIGHT, RIGHT};
//int desiredTurns[] = {LEFT, STRAIGHT, RIGHT, STRAIGHT, STRAIGHT, RIGHT, RIGHT, STRAIGHT};
int turnCount = 0;

/*
  Frequency values for different sensor checks
*/
#define passengerCheckFreq 5
#define printToLCDFreq 2000


// State Variables
int atIntersection = 0;
int turning = 0;
int turn180 = 0;
int hasPassenger = 0;
int passengerSpotted = 0;
int lostTape = 0;
int foundTape = 0; //this should be the opposite of lostTape..
int positionLost = 0; // Change to 1 if sensor data contradicts what is expected based on currentEdge[][]

void setup()
{
#include <phys253setup.txt>
  LCD.clear();
  LCD.home();

  // Attach 2 interrupts
  enableExternalInterrupt(INT1, RISING);
  enableExternalInterrupt(INT3, RISING);

  for (int i = 0; i < 4; i++) {
    motor.speed(i, 50);
  }
  delay(100);
  motor.stop_all();

  // Create Edge Matrix
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 20; j++) {
      if (theMap[i][j] != -1) {
        nodeMat[j][theMap[i][j]] = i;
      }
    }
  }

  //Just like MATLAB robotNav.m
  //create initialProfitMatrix
  for(int i = 0; i<4; i++){
  	for(int j = i; j <20; j++){
  		if (theMap[i][j] >= 0){
        initialProfitMatrix[i][j] = 100 - 8*distToDropoff[theMap[i][j]] - 6*stuckLikelyhood[theMap[i][j]];
      }
      else{
        initialProfitMatrix[i][j] = GARBAGE;
      }
  	}
  }

  initialProfitMatrix[N][7] = GARBAGE; //never go to 2

  currentEdge[0] = 0;
  currentEdge[1] = 10;

  for(int i = 0; i <4; i++){
    for(int j = 0; j < 20; j++){
      profitMatrix[i][j] = initialProfitMatrix[i][j];
    }
  }


  // Initialize important variables with stored values
  g = menuItems[0].Value;
  kp = menuItems[1].Value;
  kd = menuItems[2].Value;
  vel = menuItems[3].Value;
  tapeFollowVel = vel;
  intGain = menuItems[4].Value;

  // Home Servos
  RCServo0.write(clawOpen);
  RCServo1.write(armHome);
  // Probably should home GM7 too

  while (true) {
    currentEdge[0] = (int)((float)knob(6)/1024.0*20.0);
    currentEdge[1] = (int)((float)knob(7)/1024.0*20.0);
    LCD.clear();
    LCD.print("Press Start"); LCD.setCursor(0,1);
    LCD.print("E0: "); LCD.print(currentEdge[0]); LCD.print(" E1: "); LCD.print(currentEdge[1]);
    delay(200);
    if (startbutton())
    {
      while (true) {
        if (!startbutton()) {
          LCD.clear();
          return;
        }
        delay(100);
      }
    }
  }

}

void loop() {
  /*
    A NOTE ON FUNCTIONS IN THE MAIN LOOP:
      It is expected that functions will be called every loop iteration, so must behave accordingly
      For a function to not always be called, it must alter some variable so it will not be called next loop
      Other functions may also change this variable as appropriate
        ie. ProcessIntersection sets desiredTurn = GARBAGE after successful completion of intersection
  */

  numOfIttrs++;
  loopsSinceLastInt++;
  /*TAPE FOLLOWING*/
  //low reading == white. High reading == black tape.
  qrdVals[0] = digitalRead(q0);
  qrdVals[1] = digitalRead(q1);
  qrdVals[2] = digitalRead(q2);
  qrdVals[3] = digitalRead(q3);


  CollisionCheck();

  if (numOfIttrs%passengerCheckFreq == 0 && failedPickup == 0) {
    passengerSide = CheckForPassenger();
    if(passengerSide){
      if(!hasPassenger){
        motor.speed(LEFT_MOTOR, -1*MAX_MOTOR_SPEED);
        motor.speed(RIGHT_MOTOR, -1*MAX_MOTOR_SPEED);
        delay(20);
        motor.stop_all();
        hasPassenger = PickupPassenger(passengerSide);
        if(hasPassenger){
          //g = g*1.1;
          //intGain = intGain*1.1;
          TurnDecision();
        } else{
          failedPickup = 1; 
        }

      }else{
        passengerSpotted = true;
        profitMatrix[nodeMat[currentEdge[1]][currentEdge[0]]][currentEdge[1]] = 500; // Set profitability of current edge in both direction very high
        profitMatrix[nodeMat[currentEdge[0]][currentEdge[1]]][currentEdge[0]] = 500;
      } //TODO had a delay here and it froze the robot - sign of bigger problem???
      passengerSide = 0;
    }
  }
  // This code creates a buffer between a failed pickup attempt and a new pickup attempt, eliminating the pickup jitterbug
  if(failedPickup){
      failedPickup++;
      if(failedPickup >= 500){ //TODO: Tweak this value if needed. 
        failedPickup = 0;
      }
  }

  if(collisionDetected){
    if(!qrdVals[0] && !qrdVals[1] && !qrdVals[2] && !qrdVals[3]){
      if(switchVals[FRONT_LEFT_BUMPER]){
        ReverseLeft();
      }else if(switchVals[FRONT_RIGHT_BUMPER]){
        ReverseRight();
      }else if(switchVals[FRONT_BUMPER]){
        ReverseRight(); // TODO: Make a reverse straight/change this based on currentEdge??
      }
      //TODO: Implement check for passenger at front from dev
    }else if(switchVals[FRONT_BUMPER] || switchVals[FRONT_LEFT_BUMPER] || switchVals[FRONT_RIGHT_BUMPER]){
      // Check which way to turn based on currentEdge[1]
      switch(currentEdge[1]){
        case 0: TurnCW(); break;
        case 1: TurnCCW(); break;
        case 2: TurnCCW(); break;
        case 3: TurnCW(); break;
        case 4: TurnCCW(); break;
        case 5: TurnCCW(); break;
        case 9: TurnCW(); break;
        default: TurnCCW();
        }
    }
    for(int i = 0; i<6;i++){
      switchVals[i] = 0;
    }
    collisionDetected = false;
  }

  if(!atIntersection){
    AreWeThereYet();
  }

  if(loopsSinceLastInt == 100){
    UpdateProfitMatrix();
  } else if (loopsSinceLastInt == 300) {
    TurnDecision();
  }

  //Continue on
  if (atIntersection) {
    ProcessIntersection();
  } else { //keep tape following
    TapeFollow();
  }

  if((currentEdge[0] == 17 && currentEdge[1] == 18) || (currentEdge[0] == 18 && currentEdge[1] == 17)){
    //Going towards dropoff - count with encoders
    if(leftInitial == GARBAGE && hasPassenger){
      leftInitial = leftCount;
      rightInitial = rightCount;
    }
    if(((leftCount - leftInitial > countToDropoff) && (rightCount - rightInitial > countToDropoff))  &&  hasPassenger){
      // Have reached dropoff zone
      if(true/*(leftCount - leftInitial < countToDropoff + dropWidth) || (rightCount - rightInitial < countToDropoff + dropWidth)*/){
          // Might not need this if depending on passener positions on 17-18 edge
        motor.stop_all();
        stopTime2 = millis();
        if(stopTime2 - stopTime1 > 100){
          stopTime1 = stopTime2;
        }
        DropoffPassenger((currentEdge[0]*2-35)*-1); // 17 -> 1 or 18 -> -1
        LCD.clear();
        LCD.print(leftCount - leftInitial); LCD.print(" "); LCD.print(rightCount - rightInitial);
        leftInitial = GARBAGE;
        rightInitial = GARBAGE;
        if(passengerSpotted){
          TurnCCW();
          passengerSpotted = 0;
          UpdateProfitMatrix();
          TurnDecision();
        }
      }else{
        //turnAround(); // Don't think we'll ever get here
      }
    }


  }

  //Print useful information
  if (numOfIttrs == printToLCDFreq){ 
    PrintToLCD();
  }

  // Enter Menu if startbutton
  if (startbutton())
  {
    delay(100);
    if (startbutton())
    {
      MainMenu();
    }
  }
}

void TapeFollow() {
  if (qrdVals[1] == LOW && qrdVals[2] == LOW) {
    if(qrdVals[0] == HIGH){
      statusCountTapeFollow++;
      if(statusCountTapeFollow > 8){
        error = 12;
      }
    }else if(qrdVals[3] == HIGH){
      statusCountTapeFollow--;
      if(statusCountTapeFollow < 8){
        error = -12;
      }
    }else{ // All low
      statusCountTapeFollow = 0;
      if (pastError < 0) {
        error = -5;
      } else if (pastError > 0) {
        error = 5;
      } else if (pastError == 0) {
        // Do we need to do anything? Just go straight?
      }
    }
  } else if ( qrdVals[2] == HIGH) {
    statusCountTapeFollow = 0;
    error = -1;
  } else if (qrdVals[1] == HIGH) {
    statusCountTapeFollow = 0;
    error = 1;
  } else {
    statusCountTapeFollow = 0;
    error = 0;
  }

  if (!error == pastError) {
    recError = prevError;
    q = m;
    m = 1;
  }

  p = kp * error;
  d = (int)((float)kd * (float)(error - recError) / (float)(q + m));
  correction = (float)((p + d)*g)/10; //this was a hasty change
  avgCorrection = (avgCorrection*19+correction)/20;
  pastError = error;
  m++;
  if(!passengerSide){ // If passenger has not been seen, go forward
    motor.speed(LEFT_MOTOR, tapeFollowVel - correction);
    motor.speed(RIGHT_MOTOR, tapeFollowVel + correction);
  }
}

void PrintToLCD() {
  t2 = millis();
  loopTime = ((t2 - t1) * 1000) / printToLCDFreq;
  t1 = t2;
  numOfIttrs = 0;
  if (1/*!atIntersection*/) {
    LCD.clear();
    /*LCD.print("LT: "); LCD.print(loopTime);
    LCD.print(" i: "); LCD.print(turnCount);*/
    //LCD.print("Enc: "); LCD.print(leftCount); LCD.print(" "); LCD.print(rightCount);
    //LCD.print(hasPassenger); LCD.print("  "); LCD.print(passengerSpotted);
    LCD.print("P: "); LCD.print(profits[0]); LCD.print(" "); LCD.print(profits[1]); LCD.print(" "); LCD.print(profits[2]);  LCD.print(" "); LCD.print(profits[3]); 
    LCD.setCursor(0, 1); LCD.print("Next: "); LCD.print(currentEdge[1]); LCD.print(" Dir: "); LCD.print(desiredTurn);
  }
}

void enableExternalInterrupt(unsigned int INTX, unsigned int mode)
{
  if (INTX > 3 || mode > 3 || mode == 1) return;
  cli();
  /* Allow pin to trigger interrupts        */
  EIMSK |= (1 << INTX);
  /* Clear the interrupt configuration bits */
  EICRA &= ~(1 << (INTX*2+0));
  EICRA &= ~(1 << (INTX*2+1));
  /* Set new interrupt configuration bits   */
  EICRA |= mode << (INTX*2);
  sei();
}

ISR(INT1_vect) {rightCount++;};
ISR(INT3_vect) {leftCount++;};









