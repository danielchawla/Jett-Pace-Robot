void AreWeThereYet(){
  if ((qrdVals[0] == HIGH || qrdVals[3] == HIGH) && (qrdVals[1] == HIGH || qrdVals[2] == HIGH) /*|| 
      (qrdVals[0] == LOW && qrdVals[1] == LOW && qrdVals[2] == LOW && qrdVals[3] == LOW) */ && loopsSinceLastInt > 2000) {
      statusCount++;
    } else if (statusCount > 10) {
      statusCount-=10;
    }
    if (statusCount == 30) {
      motor.speed(LEFT_MOTOR, -1 * MAX_MOTOR_SPEED);
      motor.speed(RIGHT_MOTOR, -1 * MAX_MOTOR_SPEED);
      motor.stop_all();
      atIntersection = 1;
      statusCount = 0;
    }
    //if((qrdVals[0] == HIGH || qrdVals[3] == HIGH) && (qrdVals[1] == HIGH || qrdVals[2] == HIGH)){
    //  atIntersection++;
    //}// || (qrdVals[0] == LOW && qrdVals[1] == LOW && qrdVals[2] == LOW && qrdVals[3] == LOW);
    //one of outside sensors tape && one of inside sensors tape OR all QRDS off tape
    //if(atIntersection){ // If all QRDs lost, need to turn 180 degrees
    //turn180 = (qrdVals[0] == LOW && qrdVals[1] == LOW && qrdVals[2] == LOW && qrdVals[3] == LOW);
    //}
    if (atIntersection == 1) {
      LCD.clear();
      LCD.print("Going Straight");

      turn180 = (qrdVals[0] == LOW && qrdVals[1] == LOW && qrdVals[2] == LOW && qrdVals[3] == LOW);
    }
}


void ProcessIntersection() {
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
  countInIntersection++;

  if(turnActual == BACK){ // Need to deal with cases where QRDs cross tape or never leave tape.
    // Currently only works with dead ends
    if(!lostTape){
      motor.speed(LEFT_MOTOR,-1*MAX_MOTOR_SPEED/2);
      motor.speed(RIGHT_MOTOR,1*MAX_MOTOR_SPEED/2);
    }else{
      motor.speed(LEFT_MOTOR,-1*MAX_MOTOR_SPEED);
      motor.speed(RIGHT_MOTOR,1*MAX_MOTOR_SPEED);
    }
    if(qrdVals[0] == LOW && qrdVals[1] == LOW && qrdVals[2] == LOW && qrdVals[3] == LOW){
      lostTape = 1;
    }else if(qrdVals[1] == LOW || qrdVals[2] == LOW){
      lostTape = 0;
      atIntersection = 0;
    }
  }

  if (!turning && !(turnActual==BACK)) {
    // Check if 180 is desired.  This is always possible
    if(desiredTurn == BACK){
      turnActual == BACK; //once this executes the rest of this if statement will execute as well. this is not good. -Ryan
    }

    if (countInIntersection > maxInIntersection) {
      atIntersection = 0;
      LCD.clear();
      LCD.print("Leaving");
    }

    // Collect error values so that Tape Following continues nicely after intersection - do we really need this?
    if (qrdVals[1] == LOW && qrdVals[2] == LOW) {
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

    /*p = kp * error;
      d = (int)((float)kd * (float)(error - recError) / (float)(q + m));
      correction = p + d;
    */
    pastError = error;
    m++;
    motor.speed(LEFT_MOTOR, vel / 4 - correction);
    motor.speed(RIGHT_MOTOR, vel / 4 + correction);

    // Check if it is possible to turn left or right
    if (qrdVals[0]) {
      leftTurnPossible++;
    }
    if (qrdVals[3]) {
      rightTurnPossible++;
    }

    if (leftTurnPossible && !qrdVals[0] && rightTurnPossible < pathConfidence) {
      leftTurnPossible--;
    }
    if (rightTurnPossible && !qrdVals[3] && rightTurnPossible < pathConfidence) {
      rightTurnPossible--;
    }

    if (!qrdVals[0] && !qrdVals[3]) {
      LCD.clear(); LCD.print("STRAIGHT");
      atIntersection = 0;
    }

    // Check if all QRDs are lost
    /*if(qrdVals[0] == LOW && qrdVals[1] == LOW && qrdVals[2] == LOW && qrdVals[3] == LOW){
      countStatus++;


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
      }*/

    // Determine if we can turn the desired direction
    if (desiredTurn == LEFT){
      if(leftTurnPossible > pathConfidence) {
        turnActual = LEFT;
        turning = 1;
        qrdToCheck = q0;
        LCD.clear();
        LCD.print("Turning Left");
      }
      else{
        discrepancyInLocation = true;
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
      else{
        discrepancyInLocation = true;
      }
    }
  }

  if (turning) {
    if (loopNum == 1) {
      if (digitalRead(qrdToCheck) == LOW) {
        statusCount++;
        if (statusCount == 10) {
          loopNum = 2;
          statusCount = 0;
        }
      } else {
        statusCount = 0;
      } //one of outside is high so keep going
      motor.speed(LEFT_MOTOR, vel / 4);
      motor.speed(RIGHT_MOTOR, vel / 4);
    }
    if (loopNum == 2) {
      if (digitalRead(qrdToCheck) == HIGH) {
        statusCount++;
        if (statusCount == 10) {
          loopNum = 3;
          statusCount = 0;
        }
      } else {
        statusCount = 0;
      }
      motor.speed(LEFT_MOTOR, vel / 2 + turnActual * intGain); //minus should be plus and vise versa when turning right.
      motor.speed(RIGHT_MOTOR, vel / 2 - turnActual * intGain);
    }
    if (loopNum == 3) {
      if (digitalRead(qrdToCheck) == LOW) {
        statusCount++;
        if (statusCount == 10) {
          loopNum = 0;
          statusCount = 0;
        }
      } else {
        statusCount = 0;
      }
      motor.speed(LEFT_MOTOR, vel / 2 + turnActual * intGain / 3);
      motor.speed(RIGHT_MOTOR, vel / 2 - turnActual * intGain / 3);
    }
    if (loopNum == 0) {
      atIntersection = 0;
      pastError = turnActual * -1;
    }
    /*if (statusCount < -10) {
      motor.stop_all();
      LCD.clear(); LCD.print("Stuck turning");
      while (true) {
        delay(1000);
      }
    }*/
  }
  if (!atIntersection) { // If no longer at intersection reset apropriate variables
    // FOR TESTING
    /*
      if(countInIntersection > 20){
      desiredTurn = -2;
      }*/
    desiredTurn = -2;
    turnActual = -2;

    currentDir = (nodeMat[currentEdge[1]][currentEdge[0]] + 2) % 4; // This should probably be somewhere else, here for testing
    currentEdge[0] = currentEdge[1];
    currentEdge[1] = theMap[(currentDir + turnActual + 4) % 4][currentEdge[0]];
    if (currentEdge[1] == -1) {
      positionLost = 1;
    }

    countInIntersection = 0;
    turning = 0;
    leftTurnPossible = 0;
    rightTurnPossible = 0;

    loopsSinceLastInt = 0;

    loopNum = 1;
    lostTape = 0;
  }
}

