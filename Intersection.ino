void AreWeThereYet(){
  // Check if a situation for an intersection (one inside && one outside QRD sees tape)
  if ((qrdVals[0] == HIGH || qrdVals[3] == HIGH) && (qrdVals[1] == HIGH || qrdVals[2] == HIGH) && loopsSinceLastInt > 1000) {
    statusCount++;
    if (qrdVals[0]) {
      leftTurnPossible++;
    }
    if (qrdVals[3]) {
      rightTurnPossible++;
    }
    if (leftTurnPossible && !qrdVals[0] && leftTurnPossible < pathConfidence) {
      leftTurnPossible--;
    }
    if (rightTurnPossible && !qrdVals[3] && rightTurnPossible < pathConfidence) {
      rightTurnPossible--;
    }

  } else if (statusCount > 10) { // Decrease status count, not below 0 though
    statusCount-=10;
  }
  if (statusCount == 30) { // If status count reaches 30, enter the intersection processing code
    motor.speed(LEFT_MOTOR, -1 * MAX_MOTOR_SPEED);
    motor.speed(RIGHT_MOTOR, -1 * MAX_MOTOR_SPEED);
    motor.stop_all();
    atIntersection = 1;
    statusCount = 0;
  }
  if (atIntersection == 1) {
    LCD.clear();
    LCD.print("Going Straight");
  }
}


void ProcessIntersection() {
  motor.speed(BUZZER_PIN, MAX_MOTOR_SPEED*3/4); // Buzzer on
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
  countInIntersection++; // Increase the count in intersection


  if (!turning) { 
    /* 
      Code in this block for goint straight - 
        Jett will initially go straight until it sees it's desired turn is possible, may have already happened while detecting the intersection
    */
    if (countInIntersection > maxInIntersection) {
      // Leave the intersection if a max count is exceeded.  I don't think this happens any more - see line 117
      atIntersection = 0;
      LCD.clear();
      LCD.print("Leaving");
    }

    // Collect error values so that Tape Following continues nicely after intersection - do we really need this?
    // Only if not under leaving circumstances - not tape following yet
    if(leavingCount < 40){
      /*if (qrdVals[1] == LOW && qrdVals[2] == LOW) {
        if (pastError < 0) {
          error = -5;
        }
        if (pastError > 0) {
          error = 5;
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

      pastError = error;
      m++;*/

      motor.speed(LEFT_MOTOR, vel / 8 - correction/2);
      motor.speed(RIGHT_MOTOR, vel / 8 + correction/2); // CHANGE may need to have to set back to /4
    }


    // Check if it is possible to turn left or right
    if (qrdVals[0]) {
      leftTurnPossible++;
    }
    if (qrdVals[3]) {
      rightTurnPossible++;
    }

    if (leftTurnPossible && !qrdVals[0] && leftTurnPossible < pathConfidence) {
      leftTurnPossible--;
    }
    if (rightTurnPossible && !qrdVals[3] && rightTurnPossible < pathConfidence) {
      rightTurnPossible--;
    }


    // Check if a leaving condition is met - currently must be seen for 40 cycles
    if (!qrdVals[0] && !qrdVals[3]) {
      leavingCount++;
      LCD.clear(); LCD.print("STRAIGHT");
      if(leavingCount > 30){ // Strong condition to leave intersection - 
        /* 
          This is to handle case where one outside is lost before other one sees tape coming into a T at a weird angle 
            - Nodes 17 and 18 from the North
        */
        turnActual = STRAIGHT;

        /* 
          May need to tape follor for a little bit while in intersection to be able to handle the situation where we 
         try to go straight at an intersection where we cannot 
            - Currently will exit intersection having gone straight and we have to hope it tape follows correctly
        */
        //TapeFollow();

        atIntersection = 0; // Set at intersection to 0 to leave intersection - 
            //Have to get rid of this and uncomment next IF if we want to tape follow for a bit
      }
      /*if(leavingCount > 200){ // This is where we would determine if we have tape followed long enough to know that we successfully went straight
        atIntersection = 0;
      }*/
    }

    /* 
      This commented IF attempts to handle the case where all QRDs are lost - will only happen if we tried to go straight and couldn't
      WILL NEVER EXECUTE unless we tape follow for a bit while still in the intersection (previous ~20 lines)
    */
    // Check if all QRDs are lost
    /*if(qrdVals[0] == LOW && qrdVals[1] == LOW && qrdVals[2] == LOW && qrdVals[3] == LOW){
      statusCount++;

      if(statusCount > 10){
        // need to turn
        if(leftTurnPossible>pathConfidence){
          turnActual = LEFT;
          turning = 1;
          qrdToCheck = q0;
          loopNum = 2; // We know that outside qrd is already lost, so can jump in to loop 2 - see turning code
          LCD.clear();
          LCD.print("Turning Left");
        } else if(rightTurnPossible>pathConfidence){
          turnActual = RIGHT;
          turning = 1;
          qrdToCheck = q3;
          loopNum = 2;
          LCD.clear();
          LCD.print("Turning Right");
        } else{ // CHANGE - will get stuck here - need to handle it
          LCD.print("No straight or turn");
          while(true){}
        }
      }
    }*/

    // Determine if we can turn the desired direction
    if (desiredTurn == LEFT){
      if(leftTurnPossible > pathConfidence) {
        turnActual = LEFT; // Updates turnActual to the direction we actually attempt to turn - 
                           //This should happen everywhere we execute a turn or finish going straight, and is crucial to Navigation
        turning = 1; // Set to now avoid the going straight block of code and enter the turning block 
        qrdToCheck = q0; // The outside QRD that we want to look at
        LCD.clear();
        LCD.print("Turning Left");
      }
    }
    if (desiredTurn == RIGHT){
      if(rightTurnPossible > pathConfidence) {
        turnActual = RIGHT;
        turning = 1;
        qrdToCheck = q3;
        LCD.clear();
        LCD.print("Turning Right");
      }
    }
  }

  if (turning) {
    /*
      The turning block of code - Designed with if statements to determine the present "loop" so we can still execute code in the main loop
      3 Stages:
        Loop 1: Continue Straight - Wait until outside QRD on side we are turning towards loses the tape
        Loop 2: Start Turning - Wait until outside QRD sees the tape again - we have completed most of the 90° turn
        Loop 3: Turn slower - Waith until outside QRD loses the tape again - we are now in a good position to resume tape following
    */
    // Loop 1
    if (loopNum == 1) {
      if (digitalRead(qrdToCheck) == LOW) {
        statusCount++;
        if (statusCount == 15) { // Tape must be lost 15 cycles - this keeps us goint straight a little longer after we see the turn
                                 // May need to be adjusted when we're going faster
          loopNum = 2; // Break out of this loop if statusCount is high enough
          statusCount = 0;
        }
      } else {
        statusCount = 0; // This is really severe - maybe change to statusCount--
      } //one of outside is high so keep going
      motor.speed(LEFT_MOTOR, vel / 6);
      motor.speed(RIGHT_MOTOR, vel / 6);
    }
    // Loop 2
    if (loopNum == 2) {
      if (digitalRead(qrdToCheck) == HIGH) {
        statusCount++;
        if (statusCount == 10) { // tape seen for 5 cycles, kinda low, may need to strengthen
          loopNum = 3;
          statusCount = 0;
        }
      } else {
        statusCount = 0; // Again really severe
      }
      motor.speed(LEFT_MOTOR, vel / 3 + turnActual * intGain); //minus should be plus and vise versa when turning right.
      motor.speed(RIGHT_MOTOR, vel / 3 - turnActual * intGain);
    }
    // Loop 3
    if (loopNum == 3) {
      if (digitalRead(qrdToCheck) == LOW) {
        statusCount++;
        if (statusCount == 5) { // Again, kinda low, could strengthen
          loopNum = 0; // Loop num set to 0 to signal that we have completed the turn
          statusCount = 0;
        }
      } else {
        statusCount = 0; // Could be --
      }
      motor.speed(LEFT_MOTOR, vel / 3 + turnActual * intGain / 3);
      motor.speed(RIGHT_MOTOR, vel / 3 - turnActual * intGain / 3);
    }
    if (loopNum == 0) { // Leave intersection, set a pastError so that derivative gain is appropriate - may not be necessary but works
      atIntersection = 0;
      pastError = turnActual * -1;
    }
  }

  if (!atIntersection) { // If no longer at intersection reset apropriate variables
    motor.speed(BUZZER_PIN, 0); // Stop buzzer

    /*
      Check if we could not turn the way we wanted to - if we couldn't and decision making is working correctly, then we're lost
      Could be updated to be a bit smarter, and use what was actually seen if we went straight to further determine if we're lost
      Currently, we can always go straight even at an L or T intersection, so this is not 100% reliable but will (should) never give false positives
    */
    if(desiredTurn != turnActual){
      discrepancyInLocation = true;
    }
    for (int i = -1; i++)

    // Update the current edge based on turnActual
    currentEdge[0] = currentEdge[1];
    currentEdge[1] = theMap[(currentDir + turnActual + 4) % 4][currentEdge[0]];
    if (currentEdge[1] == -1) { // I think this is redundant with line 265 but might be a better check
      discrepancyInLocation = 1;
    }

    // Reset these values to garbage so that a new decision can be made and turnActual will be reset at the next intersection
    desiredTurn = GARBAGE;
    turnActual = GARBAGE;

    // Reset more counts and status variables - It is CRUCIAL that all variables are reset as necessary, otherwise next intersection will not work
    countInIntersection = 0;
    turning = 0;
    leftTurnPossible = 0;
    rightTurnPossible = 0;
    leavingCount = 0;
    loopNum = 1;
    loopsSinceLastInt = 0;


  }
}

