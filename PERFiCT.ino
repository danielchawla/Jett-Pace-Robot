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

/*
    Function Prototypes by File
*/
// Main
void TapeFollow(void);
void PrintToLCD(void);
// PassengerPickup
int PickupPassenger(int);
void CheckForPassenger(void);
// Intersection
void AreWeThereYet(void);
void ProcessIntersection(void);
// Decisions
void TurnDecision(void);
// Menu Functions
void MainMenu(void);
void Menu(void);
void ViewDigital(void);
void ViewAnalog(void);
void ControlArm(void);
void altMotor(void);
void PickupPassengerMain(void);

void (*menuFunctions[])() = {Menu, ViewDigital, ViewAnalog, ControlArm, PickupPassengerMain, altMotor};
int countMainMenu = 6;
const char *mainMenuNames[] = {"Change Vars", "View Digital In", "View Analog In", "Control Arm", "Pickup Passenger", "Alt Motor"};

/** Store a variable in TINAH mem*/
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
int divisors[] = {8, 8, 8, 1, 4}; //divides gains and speeds by this number


/*
  Define ALL Pins
*/
// Digital:
// Tape follwing QRDs
/*q0:far left, q1:left centre, q2right centre, q3: far right*/
int q0 = 15;
int q1 = 14;
int q2 = 13;
int q3 = 12; int qrdVals[4];
//Switches?

// Claw trip
int clawTrip = 0;

// Analog
//IR
int ArmIRpin = 0;
int leftIR = 0;   int leftIRVal = -1;   int leftIRValMax = -1;
int rightIR = 1;  int rightIRVal = -1;  int rightIRValMax = -1;

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
int lastIntersectionType;

int profitMatrix[20][20];

//edge matrix stuff
int theMap[4][20] = { // theMap[currentInd][dir] = [toIndex]
  //0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19
  { -1, -1, -1, -1, -1, -1, 1, 2, 3, -1, 0, 6, 7, 7, 8, 4, 10, 11, 14, 15}, //N
  { -1, -1, -1, -1, -1, 6, -1, 13, 9, -1, 11, 12, -1, 14, 15, -1, 17, 18, 19, -1}, //E
  {10, 6, 7, 8, 15, -1, 11, -1, 14, -1, 16, 17, 13, 12, 18, 19, -1, -1, -1, -1}, //S
  { -1, -1, -1, -1, -1, -1, 5, 12, -1, 8, -1, 10, 11, -1, 13, 14, -1, 16, 17, 18} //W
}; //dont change this
//                      0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19
int dirToDropoff[20] = {S, S, S, S, S, E, S, E, S, W, S, S, W, E, S, S, E, E, W, W}; // Direction of dropoff zone from each intersection
int bearingToDropoff[20] = {120, 160, 180, 200, 240, 120, 150, 180, 210, 240, 110, 120, 160, 200, 240, 250, 100, 100, 260, 260}; // gives bearing to dropoff from each node
int distToDropoff[20] = {4, 4, 5, 4, 4, 4, 3, 4, 3, 4, 3, 2, 3, 3, 2, 3, 2, 1, 1, 2};
int intersectionType[20]; // stores type of each intersection ie. 4-way, 4 bit boolean {NSEW} T/F values

int currentEdge[2];
int currentDir;
int possibleTurns[3] = {0}; // left, straight, right True/False values - necessary??
int desiredTurn = -2;
int turnActual = -2;
int nodeMat[20][20] = {0}; //nodeMat[fromIndex][toIndex] = dir
int countInIntersection = 0;
int maxInIntersection = 1300;
int countTurning = 0;
int maxTurning = 3000;
int leftTurnPossible = 0;
int rightTurnPossible = 0;
int intGain;
int lostCount = 0;
int qrdToCheck;
int loopNum = 1;
int statusCount = 0;
int statusLast = 0;
int pathConfidence = 2;
int loopsSinceLastInt = 0;

// Loop timing variables
unsigned long t1 = 0;
unsigned long t2 = 0;
int loopTime;

// Passenger Pickup
int SideIRMin = 400;

// Angles of straight arm and open claw
int armHome = 95;
int clawHome = 110;
int clawClose = 10;

int desiredTurns[] = {STRAIGHT, LEFT, LEFT, RIGHT, LEFT, STRAIGHT, LEFT, STRAIGHT, RIGHT, RIGHT, STRAIGHT, STRAIGHT, RIGHT, STRAIGHT, BACK}; //these are temporary and only for testing
//int desiredTurns[] = {STRAIGHT, LEFT, LEFT, RIGHT, LEFT, STRAIGHT, STRAIGHT, LEFT, STRAIGHT, RIGHT};
//int desiredTurns[] = {LEFT, STRAIGHT, LEFT, STRAIGHT, STRAIGHT, LEFT, STRAIGHT, RIGHT};
int turnCount = 0;

/*
  Frequency values for different sensor checks
*/
int passengerCheckFreq = 10;
int printToLCDFreq = 500;


// State Variables
int atIntersection = 0;
int turning = 0;
int turn180 = 0;
int hasPassenger = 0;
int lostTape = 0;
int foundTape = 0; //this should be the opposite of lostTape..
int positionLost = 0; // Change to 1 if sensor data contradicts what is expected based on currentEdge[][]

void setup()
{
#include <phys253setup.txt>
  LCD.clear();
  LCD.home();

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

  //create profitMatrix
  for(int i = 0; i<20; i++){
  	for(int j = i; j <20; j++){
  		profitMatrix[i][j] = 10 - distToDropoff[j];
  	}
  }

  // Set initial edge
  currentEdge[0] = 3;
  currentEdge[1] = 4;

  // Initialize important variables with stored values
  g = menuItems[0].Value;
  kp = menuItems[1].Value;
  kd = menuItems[2].Value;
  vel = menuItems[3].Value;
  intGain = menuItems[4].Value;

  // Home Servos
  RCServo0.write(clawHome);
  RCServo1.write(armHome);
  // Probably should home GM7 too

  LCD.print("Press Start To Begin");
  while (true) {
    delay(200);
    LCD.scrollDisplayLeft();
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
        ie. ProcessIntersection sets desiredTurn = -2 after successful completion of intersection
  */


  numOfIttrs++;
  loopsSinceLastInt++;
  /*TAPE FOLLOWING*/
  //low reading == white. High reading == black tape.
  qrdVals[0] = digitalRead(q0);
  qrdVals[1] = digitalRead(q1);
  qrdVals[2] = digitalRead(q2);
  qrdVals[3] = digitalRead(q3);

  leftIRVal = analogRead(leftIR);
  rightIRVal = analogRead(rightIR);


  //Check for passengers on either side
  if (/*numOfIttrs%passengerCheckFreq == 0*/false) {
    CheckForPassenger();
  }


  //Determine which direction to turn
  if (desiredTurn == -2) {
    TurnDecision();
  }

  if(!atIntersection){
    AreWeThereYet();
    /*LCD.clear();
    LCD.print("Checking for Int");*/
  }

  //Continue on
  if (atIntersection) {
    ProcessIntersection();
  } else { //keep tape following
    TapeFollow();
  }


  //Print useful information
  if (numOfIttrs == printToLCDFreq) {
    //LCD.clear();
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
    if (pastError < 0) {
      error = -5;
    } else if (pastError > 0) {
      error = 5;
    } else if (pastError == 0) {
      // Do we need to do anything? Just go straight?
    }
  } else if ( qrdVals[1] == LOW) {
    error = -1;
  } else if (qrdVals[2] == LOW) {
    error = 1;
  } else {
    error = 0;
  }

  if (!error == pastError) {
    recError = prevError;
    q = m;
    m = 1;
  }

  p = kp * error;
  d = (int)((float)kd * (float)(error - recError) / (float)(q + m));
  correction = p + d;

  pastError = error;
  m++;
  motor.speed(0, vel - correction);
  motor.speed(1, vel + correction);
}

void PrintToLCD() {
  t2 = millis();
  loopTime = ((t2 - t1) * 1000) / printToLCDFreq;
  t1 = t2;
  numOfIttrs = 0;
  if (1/*!atIntersection*/) {
    LCD.clear();
    LCD.print("LT: "); LCD.print(loopTime);
    LCD.print(" i: "); LCD.print(turnCount);
    LCD.setCursor(0, 1); LCD.print("Next: "); LCD.print(currentEdge[1]); LCD.print(" Dir: "); LCD.print(desiredTurn);
  }
}








