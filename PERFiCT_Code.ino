#include <avr/EEPROM.h> //TINAH mem
#include <phys253.h> //ask Jon
#include <LiquidCrystal.h> //LCD funcs

#define N 0
#define E 1
#define S 2
#define W 3
#define STRAIGHT 0
#define RIGHT 1
#define BACK -2
#define LEFT -1

void MainMenu(void);
void Menu(void);
void ViewDigital(void);
void ViewAnalog(void);
void ControlArm(void);
int PickupPassenger(int);
void PickupPassengerMain(void);
void altMotor(void);

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
int divisors[] = {16, 16, 16, 1, 4}; //divides gains and speeds by this number


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

//NAV VARIABLES
double topIR0, ir0 = 0;
double topIR1, ir1 = 1;
double topIR2, ir2 = 2;
int directionOfDropZone; // 0 to 359 degrees (bearings).
int strongest, secondStrongest; //signals from topIRs (0,1,2)
double strongestVal, secondStrongestVal;
int offset;
//edge matrix stuff
int theMap[4][20] = { // theMap[currentInd][dir] = [toIndex]
  //0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18  19
  { -1, 0,  1,  11, 8,  6,  7,  -1, 14, 12, 12, 17, 13, -1, 16, -1, -1, 19, -1, -1}, //N
  { -1, 11, 3,  4,  5,  -1, -1, -1, 6,  8,  -1, 10, 9,  -1, 15, -1, -1, -1, 17, -1}, //E
  {  1, 2,  -1, -1, -1, -1, 5,  6,  4,  10, 9,  3,  -1, 12, 8,  -1, 14, 11, -1, 17}, //S
  { -1, -1, -1, 2,  3,  4,  8,  -1, 9,  -1, 11, 1,  10, -1, -1, 14, -1, 18, -1, -1} //W
}; //dont change this   
// Need to initialize the following two arrays
int intersectionType[20] = {2, 14, 12, 13, 13, 9, 11, 2, 15, 14, 11, 15, 13, 2, 14, 1, 2, 11, 4, 2}; // stores type of each intersection ie. 4-way, 4 bit boolean {NSEW} T/F values
int dirToDropoff[20] = {}; // Direction of dropoff zone from each intersection

int currentEdge[2];
int currentDir;
int dirPrev = 0;
int possibleTurns[3] = {0}; // left, straight, right True/False values - necessary??
int desiredTurn = -2;
int turnActual;
int nodeMat[20][20] = {0}; //nodeMat[fromIndex][toIndex] = dir
int countInIntersection = 0;
int maxInIntersection = 4000;
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
int pathConfidence = 3;
int loopsSinceLastInt = 0;

// Loop timing variables
unsigned long t1 = 0;
unsigned long t2 = 0;
int loopTime;

// Angles of straight arm and open claw
int armHome = 95;
int clawHome = 110;
int clawClose = 10;

int desiredTurns[] = {LEFT, STRAIGHT, RIGHT, STRAIGHT, STRAIGHT, RIGHT, STRAIGHT, RIGHT};
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
int foundTape = 0;
int positionLost = 0; // Change to 1 if sensor data contradicts what is expected based on currentEdge[][]

void setup()
{
  #include <phys253setup.txt>
  LCD.clear();
  LCD.home();

  for(int i = 0; i<4;i++){
    motor.speed(i,50);
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

  // Set initial edge
  currentEdge[0] = 0;
  currentEdge[1] = 1;

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

/*
  Check for passengers on either side
*/

  if(/*numOfIttrs%passengerCheckFreq == 0*/false){
    // Check left side
    if(leftIRVal > leftIRValMax){
      leftIRValMax = leftIRVal;
    }else if(leftIRVal < leftIRValMax-10){
      // Stop motors and pick up passenger
      motor.speed(0,-255);
      motor.speed(0,-255);
      delay(100);
      motor.speed(0,0);
      motor.speed(0,0);
      hasPassenger = PickupPassenger(1);
      leftIRValMax = -1;
    }
    // Check right side
    if(rightIRVal > rightIRValMax){
      // Stop motors and pick up passenger
      rightIRValMax = rightIRVal;
    }else if(rightIRVal < rightIRValMax-10){
      motor.speed(0,-255);
      motor.speed(0,-255);
      delay(100);
      motor.speed(0,0);
      motor.speed(0,0);
      hasPassenger = PickupPassenger(-1);
      rightIRValMax = -1;
    }
  }
  
  /*
    Determine features of the next intersection and possible directions to turn
  */
  if (possibleTurns[0] == 0 && possibleTurns[1] == 0 && possibleTurns[1] == 0){
    currentDir = nodeMat[currentEdge[0]][currentEdge[1]];

    for(int i=0;i<3;i++){
      possibleTurns[i] = 0;
    }
    for (int i = 0; i < 4; i++) {
      if (theMap[i][currentEdge[1]] != -1) {
        if (currentDir - i == -3 || currentDir - i == 1) {  // Can turn Left
          possibleTurns[0] = 1;
        }
        if (currentDir - i == 3 || currentDir - i == -1) {  // Can turn Right
          possibleTurns[2] = 1;
        }
        if (currentDir - i == 0) {                          // Can go Straight
          possibleTurns[1] = 1;
        }
      }
    }
  }

  /*
    Determine which direction to turn
  */
  if (desiredTurn == -2) {
    topIR0 = analogRead(ir0);
    topIR1 = analogRead(ir1);
    topIR2 = analogRead(ir2);
    if (topIR0 > topIR1 && topIR0 > topIR2) {
      strongest = ir0;
      if (topIR1 > topIR2){
        secondStrongest = ir1;
      }
      else{
        secondStrongest = ir2;
      }
    }
    else if (topIR1 > topIR2 && topIR1 > topIR0) {
      strongest = ir1;
      if (topIR0 > topIR2){
        secondStrongest = ir0;
      }
      else{
        secondStrongest = ir2;
      }
    }
    else {
      strongest = ir2;
      if (topIR0 > topIR1){
        secondStrongest = ir0;
      }
      else{
        secondStrongest = ir1;
      }
    }
    directionOfDropZone = 120 * strongest;
    secondStrongestVal = analogRead(secondStrongest);
    strongestVal = analogRead(strongest);
    offset = 120.0 * (float)secondStrongestVal / (float)(secondStrongestVal + strongestVal); // Need to be cast as doubles? - analogRead gives int
    if ( (strongest + 1) % 3 == secondStrongest ){
      directionOfDropZone = (directionOfDropZone + offset+360) % 360;
    }
    else{
      directionOfDropZone = (directionOfDropZone - offset+360) % 360;
    }

    // For testing, turn left, right, straight, left ...
    desiredTurn = desiredTurns[turnCount];//(dirPrev+1)%3;
    turnCount++;
    dirPrev = desiredTurn; 
  }

  /*
    Check if at an intersection if not already
  */

  if(atIntersection < 6 && loopsSinceLastInt > 5000){
    if((qrdVals[0] == HIGH || qrdVals[3] == HIGH) && (qrdVals[1] == HIGH || qrdVals[2] == HIGH) || (qrdVals[0] == LOW && qrdVals[1] == LOW && qrdVals[2] == LOW && qrdVals[3] == LOW)){
      atIntersection++;
    }// || (qrdVals[0] == LOW && qrdVals[1] == LOW && qrdVals[2] == LOW && qrdVals[3] == LOW);
     //one of outside sensors tape && one of inside sensors tape OR all QRDS off tape
    //if(atIntersection){ // If all QRDs lost, need to turn 180 degrees
      //turn180 = (qrdVals[0] == LOW && qrdVals[1] == LOW && qrdVals[2] == LOW && qrdVals[3] == LOW);
    //}
    if(atIntersection > 5){
      LCD.clear();
      LCD.print("Going Straight");
      turn180 = (qrdVals[0] == LOW && qrdVals[1] == LOW && qrdVals[2] == LOW && qrdVals[3] == LOW);
    }
  }

  /*
    TAKE ACTION AT INTERSECTION
    When you come to and intersection there are 5 posibilities:
      1. It is a dead end - must make 180 degree turn
      2. You want to go straight, and this is an option - continue in straight line
      3. You want to go straight, this is not an option - need to make a turn (random direction?)
      4. You want to turn in a specific direction, and this is an option - make the turn successfully
      5. You want to turn in a specific direction, this is not an option 
        5a. Go straight if possible
        5b. Turn in other direction if necessary

    General approach to intersections
      • Make 180 turn if necessary
        • Otherwise continue straight
          • Write same speeds to motors for predefined number of loops, then continue tape following after intersection
      • If outside QRD identifies option to turn in desired direction, make the turn
      • If all QRDs are lost while in the intersection and not turning, going straight is not an option - turn in one possible direction
        • Possible directions are the outside QRDs that have seen tape while in the intersection - these must be recorded
        • If both directions are possible, choose which direction to continue at random
          • This is probably only relevant in testing, where directions to turn are random
  */
  if (atIntersection > 5) {
    countInIntersection++;
    

    // Check if it is possible to turn left or right
    if(!turning){
      if(countInIntersection > maxInIntersection){
        atIntersection = 0;
      }
      // Collect error values so that Tape Following continues nicely after intersection - do we really need this?
      if (qrdVals[1] == LOW && qrdVals[2] == LOW) {
        if (pastError < 0) {
          error = -5;
        }
        if (pastError > 0) {
          error = 5;
        }
      }else if ( qrdVals[1] == LOW) {
        error = -1;
      }else if (qrdVals[2] == LOW) {
        error = 1;
      }else {
        error = 0;
      }

       if (!error == pastError) {
        recError = prevError;
        q = m;
        m = 1;
      }

      // Write same speed to both motors
      motor.speed(0,vel/2);
      motor.speed(1,vel/2);
      
      if(qrdVals[0]){      
        leftTurnPossible++;
      }
      if(qrdVals[3]){
        rightTurnPossible++;
      }

      if(leftTurnPossible && !qrdVals[0] && rightTurnPossible < pathConfidence){
        leftTurnPossible--;
      }
      if(rightTurnPossible && !qrdVals[3] && rightTurnPossible < pathConfidence){
        rightTurnPossible--;
      }

      // Check if all QRDs are lost
      if(qrdVals[0] == LOW && qrdVals[1] == LOW && qrdVals[2] == LOW && qrdVals[3] == LOW){
        motor.stop_all();
        // need to turn
        if(leftTurnPossible>pathConfidence){
          turnActual = LEFT;
          turning = 1;
          qrdToCheck = q0;
          LCD.clear();
          LCD.print("Turning Left");
        } else if(rightTurnPossible>pathConfidence){
          turnActual = RIGHT;
          turning = 1;
          qrdToCheck = q3;
          LCD.clear();
          LCD.print("Turning Right");  
        } else{ // reached dead end
          // 180 turn
        }
      }

       // Determine if we can turn the desired direction
      if(desiredTurn == LEFT && leftTurnPossible > pathConfidence){
        turnActual = LEFT;
        turning = 1;
        qrdToCheck = q0;
        LCD.clear();
        LCD.print("Turning Left");
      }
      if(desiredTurn == RIGHT && rightTurnPossible > pathConfidence){
        turnActual = RIGHT;
        turning = 1;
        qrdToCheck = q3;
        LCD.clear();
        LCD.print("Turning Right");
      }
    }

    if(turning){
      if(loopNum == 1){
        if(digitalRead(qrdToCheck) == LOW){
          statusCount++;
          if(statusCount == 3){
            loopNum = 2;
            statusCount = 0;
          }
        }else{
          statusCount = 0;
        } //one of outside is high so keep going 
        motor.speed(0,vel/2);
        motor.speed(1,vel/2);
      }
      if(loopNum == 2){
        if(digitalRead(qrdToCheck) == HIGH){
          statusCount++;
          if(statusCount == 3){
            loopNum = 3;
            statusCount = 0;
          }
        }else{
          statusCount = 0;
        }
        motor.speed(0,vel/2 + turnActual*intGain); //minus should be plus and vise versa when turning right.
        motor.speed(1,vel/2 - turnActual*intGain);
        }
      if(loopNum == 3){
        if(digitalRead(qrdToCheck) == LOW){
          statusCount++;
          if(statusCount == 3){
            loopNum = 0;
            statusCount = 0;
          }
        }else{
          statusCount = 0;
        }
        motor.speed(0,vel/2 + turnActual*intGain/3);
        motor.speed(1,vel/2 - turnActual*intGain/3);
      }
      if(loopNum == 0){
        atIntersection = 0;
        pastError = turnActual*-1;
      }
      if(statusCount < -10){
        motor.stop_all();
        LCD.clear();LCD.print("Stuck turning");
        while(true){
          delay(1000);
        }
      }
    }
    if(!atIntersection){ // If no longer at intersection reset apropriate variables
      // FOR TESTING
      desiredTurn = -2;
      
      // Need to change q if robot turned???
      
      currentDir = (nodeMat[currentEdge[1]][currentEdge[0]]+2)%4; // This should probably be somewhere else, here for testing
      currentEdge[0] = currentEdge[1]; 
      currentEdge[1] = theMap[(currentDir + turnActual + 4)%4][currentEdge[0]];
      if(currentEdge[1]==-1){
        positionLost = 1;
      }

      countInIntersection = 0;
      turning = 0;
      leftTurnPossible = 0;
      rightTurnPossible = 0;

      loopsSinceLastInt = 0;

      loopNum = 1;
    }
  } else{  //keep tape following

    if (qrdVals[1] == LOW && qrdVals[2] == LOW) {
      if (pastError < 0) {
        error = -5;
      }else if (pastError > 0) {
        error = 5;
      }else if (pastError == 0){
        // Do we need to do anything? Just go straight?
      }
    }else if ( qrdVals[1] == LOW) {
      error = -1;
    }else if (qrdVals[2] == LOW) {
      error = 1;
    }else {
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


  if (numOfIttrs == printToLCDFreq) {
    t2 = millis();
    loopTime = ((t2-t1)*1000)/printToLCDFreq;
    t1 = t2;
    numOfIttrs = 0;
    if(!atIntersection){
      LCD.clear();
      LCD.print("LoopTime: "); LCD.print(loopTime);
      LCD.setCursor(0,1); LCD.print("Next: "); LCD.print(currentEdge[1]);
    }
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



void MainMenu()
{
  motor.stop_all();
  LCD.clear(); LCD.home();
  LCD.print("Entering Main Menu");
  delay(500);
  LCD.clear();
  int menuIndex = countMainMenu - knob(6) * (countMainMenu) / 1024 - 1;
  int topIndex;
  if (menuIndex < countMainMenu - 1) {
    topIndex = menuIndex;
  } else {
    topIndex = menuIndex - 1;
  }
  int cursorPosition = menuIndex - topIndex;
  LCD.print(mainMenuNames[topIndex]); LCD.setCursor(0, 1); LCD.print(mainMenuNames[topIndex + 1]);
  LCD.setCursor(0, cursorPosition);
  LCD.cursor();
  while (true)
  {
    menuIndex = countMainMenu - knob(6) * (countMainMenu) / 1024 - 1;
    if (cursorPosition != menuIndex - topIndex)
    {
      LCD.clear();
      if (menuIndex > topIndex + 1) {
        topIndex++;
      }
      if (menuIndex < topIndex) {
        topIndex--;
      }
      cursorPosition = menuIndex - topIndex;
      LCD.print(mainMenuNames[topIndex]); LCD.setCursor(0, 1); LCD.print(mainMenuNames[topIndex + 1]);
      LCD.setCursor(0, cursorPosition);
    }
    delay(100);
    if (startbutton())
    {
      delay(100);
      if (startbutton()) {
        LCD.noCursor();
        (*menuFunctions[menuIndex])();
        LCD.clear();
        LCD.print(mainMenuNames[topIndex]); LCD.setCursor(0, 1); LCD.print(mainMenuNames[topIndex + 1]);
        LCD.setCursor(0, cursorPosition);
        LCD.cursor();
      }
    }
    if (stopbutton())
    {
      delay(100);
      if (stopbutton())
      {
        LCD.clear(); LCD.home(); LCD.noCursor();
        LCD.print("Leaving menu");
        delay(500);
        return;
      }
    }
  }
}

void Menu()
{
  LCD.clear(); LCD.home();
  LCD.print("Entering menu");
  delay(500);

  while (true)
  {
    /* Show MenuItem value and knob value */
    int menuIndex = knob(6) * (MenuItem::MenuItemCount) / 1024;
    LCD.clear(); LCD.home();
    LCD.print(menuItems[menuIndex].Name); LCD.print(" "); LCD.print(menuItems[menuIndex].Value);
    LCD.setCursor(0, 1);
    LCD.print("Set to "); LCD.print(knob(7) / divisors[menuIndex]); LCD.print("?");
    delay(100);

    /* Press start button to save the new value */
    if (startbutton())
    {
      delay(200);
      if (startbutton())
      {
        menuItems[menuIndex].Value = knob(7) / divisors[menuIndex];
        menuItems[menuIndex].Save();
        delay(250);
      }
    }

    /* Press stop button to exit menu */
    if (stopbutton())
    {
      delay(100);
      if (stopbutton())
      {
        LCD.clear(); LCD.home();
        LCD.print("Leaving menu");
        g = menuItems[0].Value;
        kp = menuItems[1].Value;
        kd = menuItems[2].Value;
        vel = menuItems[3].Value;
        intGain = menuItems[4].Value;
        delay(500);

        return;
      }
    }
  }
}

void ViewDigital()
{
  LCD.clear(); LCD.print("Viewing Digital");
  delay(500);
  LCD.clear();
  while (true)
  {
    int index = knob(6) * 3 / 1024;
    if (index == 0)
    {
      LCD.print("D0:"); LCD.print(digitalRead(0)); LCD.print(" D1:"); LCD.print(digitalRead(1)); LCD.print(" D2:"); LCD.print(digitalRead(2));
      LCD.setCursor(0, 1);
      LCD.print("D3:"); LCD.print(digitalRead(3)); LCD.print(" D4:"); LCD.print(digitalRead(4)); LCD.print(" D5:"); LCD.print(digitalRead(5));


    } else if (index == 1)
    {
      LCD.print("D6:"); LCD.print(digitalRead(6)); LCD.print(" D7:"); LCD.print(digitalRead(7)); LCD.print(" D8:"); LCD.print(digitalRead(8));
      LCD.setCursor(0, 1);
      LCD.print("D9:"); LCD.print(digitalRead(9)); LCD.print(" D0:"); LCD.print(digitalRead(10)); LCD.print(" D1:"); LCD.print(digitalRead(11));
    } else if (index == 2)
    {
      LCD.print("D2:"); LCD.print(digitalRead(12)); LCD.print(" D3:"); LCD.print(digitalRead(13)); LCD.print(" D4:"); LCD.print(digitalRead(14));
      LCD.setCursor(0, 1);
      LCD.print("D5:"); LCD.print(digitalRead(15));
    }

    delay(100);
    LCD.clear();
    if (stopbutton())
    {
      LCD.clear(); LCD.home();
      LCD.print("Leaving menu");
      delay(500);
      return;
    }
  }
}

void ViewAnalog()
{
  LCD.clear(); LCD.print("Viewing Analog");
  delay(500);
  LCD.clear();
  while (true)
  {
    int index = knob(6) * 2 / 1024;
    if (index == 0)
    {
      LCD.print("A0:"); LCD.print(analogRead(0)); LCD.setCursor(8, 0); LCD.print("A1:"); LCD.print(analogRead(1));
      LCD.setCursor(0, 1);
      LCD.print("A2:"); LCD.print(analogRead(2)); LCD.setCursor(8, 1); LCD.print("A3:"); LCD.print(analogRead(3));
    } else if (index == 1)
    {
      LCD.print("A4:"); LCD.print(analogRead(4)); LCD.setCursor(8, 0); LCD.print("A5:"); LCD.print(analogRead(5));
      LCD.setCursor(0, 1);
      LCD.print("A6:"); LCD.print(analogRead(6)); LCD.setCursor(8, 1); LCD.print("A7:"); LCD.print(analogRead(7)) ;
    }

    delay(100);
    LCD.clear();
    if (stopbutton())
    {
      LCD.clear(); LCD.home();
      LCD.print("Leaving menu");
      delay(500);
      return;
    }
  }
}

void ControlArm()
{
  int armAngle = armHome;
  int clawAngle = 0;
  int spd = 0;
  int count = 0;
  LCD.clear(); LCD.print("Controlling Arm");
  delay(500);
  LCD.clear();
  LCD.print("Press Start to begin");
  while (true) {
    if (startbutton()) {
      break;
    }
  }
  while (true) {
    clawAngle = (int)(knob(7) * 110.0 / 1024.0);
    armAngle = (int)(knob(6) * 190.0 / 1024.0);
    spd =  -255 + (int)(knob(6) * 512.0 / 1024.0);

    RCServo0.write(clawAngle);

    if (!startbutton() && !stopbutton()) {
      motor.speed(2, 0);
      RCServo1.write((int)(armAngle));
    }

    else if (stopbutton()) {
      motor.speed(2, spd);
    }

    if (count == 2000) {
      count = 0;
      LCD.clear();
      LCD.print("Claw: "); LCD.print(clawAngle);
      LCD.setCursor(0, 1);
      LCD.print("Arm: "); LCD.print(armAngle); LCD.print(" Spd: "); LCD.print(spd);
    }


    if (stopbutton() && !startbutton())
    {
      RCServo0.write(clawHome);
      RCServo1.write(armHome);
      LCD.clear(); LCD.home();
      LCD.print("Leaving Arm Control");
      delay(50);
      return;
    }
    count++;
  }

}

int PickupPassenger(int side){ // side=1 if on left, side=-1 if on right
  int range = 80;
  int tripThresh = 800;
  int armDelay = 15;
  int maxIR = -1;
  int newIR = -1;
  int finalI = range-1;

  RCServo0.write(clawHome);
  RCServo1.write(armHome);

  LCD.clear(); LCD.print("Picking Up");
  
  // Position Arm
  for(int i = 0;i<=range;i++){
    RCServo1.write(armHome+i*side);
    delay(armDelay);
    newIR = analogRead(ArmIRpin);
    if(newIR > maxIR){
      maxIR = newIR;
    }else if(newIR < maxIR-10){
      finalI = i;
      break;
    }
    LCD.clear(); LCD.print("IR:  ");LCD.print(newIR);
    LCD.setCursor(0,1); LCD.print("Max: ");LCD.print(maxIR);
  }

  // Position Claw
  motor.speed(2, 150);
  for(int i=0; i<2000;i++){ // Will need to change this
    if(!digitalRead(clawTrip)){
      break;
    }
    delay(1);
  }
  motor.speed(2,0);

  // Close Claw
  RCServo0.write(clawClose);
  delay(1000);

  // Retract Claw
  motor.speed(2, -150);
  delay(400);
  // Home Arm
  for(int i = 0;i<=finalI;i++){
    RCServo1.write(armHome+(finalI-i)*side);
    delay(armDelay);
  }

  if(1600 - range*armDelay > 0){
    delay(1600 - range*armDelay);
  }
  motor.speed(2,0);

  // Home Claw
  RCServo0.write(clawHome);
  delay(1000); 
}

void PickupPassengerMain(void){
  LCD.clear(); LCD.print("Press start");
  LCD.setCursor(0,1); LCD.print("Pickup on Right");
  delay(500);
    
  while(true){
    if(startbutton()){
      break;
    }
    delay(50);
  }
  PickupPassenger(-1);
  
  LCD.clear(); LCD.print("Press start");
  LCD.setCursor(0,1); LCD.print("Pickup on Left");
  delay(500);
    
  while(true){
    if(startbutton()){
      break;
    }
    delay(50);
  }
  PickupPassenger(1);
}

void altMotor(void){
  int i = 1;
  while(true){
    motor.speed(1,200*i);
    delay(2000);
    i = i*-1;
  }
}


