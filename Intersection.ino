void detectIntersection(){
  if ((qrdVals[0] == HIGH || qrdVals[3] == HIGH) && (qrdVals[1] == HIGH || qrdVals[2] == HIGH) && loopsSinceLastInt > 600) {
    //statusCount++;
    if (qrdVals[0]) {
      leftTurnPossible+=2;
    }
    if (qrdVals[3]) {
      rightTurnPossible+=2;
    }
    if (leftTurnPossible && !qrdVals[0] && leftTurnPossible < PATHCONFIDENCE) {
      leftTurnPossible--;
    }
    if (rightTurnPossible && !qrdVals[3] && rightTurnPossible < PATHCONFIDENCE) {
      rightTurnPossible--;
    }

  }
  if (leftTurnPossible > PATHCONFIDENCE || rightTurnPossible > PATHCONFIDENCE) { // Makes harder to see intersection
    motor.speed(LEFT_MOTOR, -1 * MAX_MOTOR_SPEED);
    motor.speed(RIGHT_MOTOR, -1 * MAX_MOTOR_SPEED);
    motor.stop_all();
    atIntersection = 1;
    statusCount = 0;
  }
  if (atIntersection == 1) {
    if(currentEdge[1] == 8 && currentEdge[0] == 14){
      avgCorrection = 0;
    }
    LCD.clear();
    LCD.print("Going Straight");

    turn180 = (qrdVals[0] == LOW && qrdVals[1] == LOW && qrdVals[2] == LOW && qrdVals[3] == LOW);

    rightDiff = rightCount - rightEncoderAtLastInt;
    leftDiff = leftCount - leftEncoderAtLastInt;
  }
}


void processIntersection() {
  motor.speed(BUZZER_PIN, MAX_MOTOR_SPEED*3/4);
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

  if (!turning) {

    // Collect error values so that Tape Following continues nicely after intersection - do we really need this?
    // Only if not under leaving circumstances - not tape following yet
    if(leavingCount < 10){
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

      pastError = error;
      m++;
        motor.speed(LEFT_MOTOR, vel / 4 - avgCorrection*1);
        motor.speed(RIGHT_MOTOR, vel / 4 + avgCorrection*1);
    }
    // Check if it is possible to turn left or right
    if (qrdVals[0]) {
      leftTurnPossible+=2;
    }
    if (qrdVals[3]) {
      rightTurnPossible+=2;
    }

    if (leftTurnPossible && !qrdVals[0] && leftTurnPossible < PATHCONFIDENCE) {
      leftTurnPossible--;
    }
    if (rightTurnPossible && !qrdVals[3] && rightTurnPossible < PATHCONFIDENCE) {
      rightTurnPossible--;
    }

    if (!qrdVals[0] && !qrdVals[3] && !tapeFollowCountInInt) {
      leavingCount++;
      LCD.clear(); LCD.print("STRAIGHT");
      if(leavingCount > 50){ 
        tapeFollowCountInInt = 1;
        turnActual = STRAIGHT;
      }
    }

    if(tapeFollowCountInInt){
      tapeFollowCountInInt++;
      tapeFollow();
      if((tapeFollowCountInInt > 600 && !(currentEdge[1] == 11 && currentEdge[0] == 10)) || tapeFollowCountInInt > 800){ // previously 600
        
        // need to turn
        if(rightTurnPossible>=PATHCONFIDENCE && defaultTurn == RIGHT){
          turnActual = RIGHT;
          turning = 1;
          qrdToCheck = q3;
          loopNum = 2;
          LCD.clear();
          LCD.print("Turning Right");
        }else if(leftTurnPossible>=PATHCONFIDENCE){ // and defaultTurn == LEFT || right turn !possible
          turnActual = LEFT;
          turning = 1;
          qrdToCheck = q0;
          loopNum = 2;
          LCD.clear();
          LCD.print("Turning Left");
        } else{ // Turn Right if right turn was possible (will only get here if defaultTurn was LEFT and left turn was not possible, assuming one turn is always possible)
          turnActual = RIGHT;
          turning = 1;
          qrdToCheck = q3;
          loopNum = 2;
          LCD.clear();
          LCD.print("Turning Right");
        }
      }
    }

      //RYANS NEW CODE
    if((qrdVals[0] == HIGH || qrdVals[1] == HIGH || qrdVals[2] == HIGH || qrdVals[3] == HIGH) && tapeFollowCountInInt > 200){
      noStraightCount+=3; //this should be "straighCount" or equivalent
      
      if(noStraightCount >= 50){
        atIntersection = false; //we can turn straight! exit the intersection and keep tape following
      }
      else if(noStraightCount){
         noStraightCount--;
      }
    }


    // Determine if we can turn the desired direction
    if (desiredTurn == LEFT){
      if(leftTurnPossible > PATHCONFIDENCE) {
        turnActual = LEFT;
        turning = 1;
        qrdToCheck = q0;
        LCD.clear();
        LCD.print("Turning Left");
      }
    }
    if (desiredTurn == RIGHT){
      if(rightTurnPossible > PATHCONFIDENCE) {
        turnActual = RIGHT;
        turning = 1;
        qrdToCheck = q3;
        LCD.clear();
        LCD.print("Turning Right");
      }
    }
  }

  if (turning) {
    if (loopNum == 1) {
      if (digitalRead(qrdToCheck) == LOW) {
        statusCount++;
        if (statusCount == 15) {
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
        if (statusCount == 5) {
          loopNum = 3;
          statusCount = 0;
        }
      } else {
        statusCount = 0;
      }
      motor.speed(LEFT_MOTOR, vel / 5 + turnActual * intGain); //minus should be plus and vise versa when turning right.
      motor.speed(RIGHT_MOTOR, vel / 5 - turnActual * intGain);
    }
    if (loopNum == 3) {
      if (digitalRead(qrdToCheck) == LOW) {
        statusCount++;
        if (statusCount == 5) {
          loopNum = 0;
          statusCount = 0;
        }
      } else {
        statusCount = 0;
      }
      motor.speed(LEFT_MOTOR, vel / 3 + turnActual * intGain / 3);
      motor.speed(RIGHT_MOTOR, vel / 3 - turnActual * intGain / 3);
    }
    if (loopNum == 0) {
      atIntersection = 0;
      pastError = turnActual * -1;
    }
  }

  if (!atIntersection) { // If no longer at intersection reset apropriate variables
    resetIntersection();
  }
}

void resetIntersection(){
    /*
      Check if we could not turn the way we wanted to - if we couldn't and decision making is working correctly, then we're lost
      Could be updated to be a bit smarter, and use what was actually seen if we went straight to further determine if we're lost
      Currently, we can always go straight even at an L or T intersection, so this is not 100% reliable but will (should) never give false positives
    */
    rightEncoderAtLastInt = rightCount;
    leftEncoderAtLastInt = leftCount;
    if(currentEdge[1] != 7){
      if(desiredTurn != turnActual /*&& currentEdge[1] != 11*/){
        discrepancyInLocation = true;
      } else{
        if(leftTurnPossible >= PATHCONFIDENCE){
          if(theMap[(currentDir + LEFT + 4) % 4][currentEdge[1]] == -1){
            discrepancyInLocation = true;
          }
        }
        if(rightTurnPossible >= PATHCONFIDENCE){
          if(theMap[(currentDir + RIGHT + 4) % 4][currentEdge[1]] == -1){
            discrepancyInLocation = true;
          }
        }
        if(turnActual == STRAIGHT && currentEdge[1] != 14 && currentEdge[1] != 8){//HACK
          if(leftTurnPossible <PATHCONFIDENCE){
            if(theMap[(currentDir + LEFT + 4) % 4][currentEdge[1]] != -1){
              discrepancyInLocation = true;
            }
          }
          if(rightTurnPossible < PATHCONFIDENCE){
            if(theMap[(currentDir + RIGHT + 4) % 4][currentEdge[1]] != -1){
              discrepancyInLocation = true;
            }
          }
        }
      }
    }

    motor.speed(BUZZER_PIN, 0);
    if(desiredTurn != turnActual && currentEdge[1] != 7){
      discrepancyInLocation = true;
    }

    // Update the current edge based on turnActual
    if(!discrepancyInLocation){
      currentEdge[0] = currentEdge[1];
      currentEdge[1] = theMap[(currentDir + turnActual + 4) % 4][currentEdge[0]];
    } else{
      currentEdge[0] = GARBAGE;
      currentEdge[1] = GARBAGE;
    }

    if(discrepancyInLocation){
      motor.speed(BUZZER_PIN, MAX_MOTOR_SPEED/5);
      checkToSeeIfWeKnowWhereWeAre();//this is called right after an intersection
    }
    pastAction = turnActual;
    if(turnActual != STRAIGHT && turnActual != GARBAGE){
      pastTurn = turnActual;
    }
    desiredTurn = GARBAGE;
    turnActual = GARBAGE;

    if(!discrepancyInLocation){
      defaultTurn = defaultTurnFromNode[currentEdge[0]];
    }

    countInIntersection = 0;
    turning = 0;
    leftTurnPossible = 0;
    rightTurnPossible = 0;
    leavingCount = 0;

    loopsSinceLastInt = 0;
    //tapeFollowVel = vel;
    loopNum = 1;
    lostTape = 0;
    noStraightCount = 0;
    tapeFollowCountInInt = 0;
    avgCorrection = 0;
    if(slowedDown){
      tapeFollowVel = vel;
      slowedDown = false;
    }
}
void checkToSeeIfWeKnowWhereWeAre(void){
  //Checks to see if we are going along the straight edge by the drop off.


  if(leftDiff > CURVEOUTSIDECOUNT && rightDiff < leftDiff*0.7){
    motor.stop_all(); LCD.clear(); LCD.print("On 12-7"); delay(200);
    profitMatrix[nodeMat[13][12]][13] = 0;
    profitMatrix[nodeMat[12][13]][12] = 0;
    currentEdge[0] = 12;
    currentEdge[1] = 7;
    discrepancyInLocation = false;
  } else if(rightDiff > CURVEOUTSIDECOUNT && leftDiff < rightDiff*0.7){
    motor.stop_all(); LCD.clear(); LCD.print("On 13-7"); delay(200);
    profitMatrix[nodeMat[13][12]][13] = 0;
    profitMatrix[nodeMat[12][13]][12] = 0;
    currentEdge[0] = 13;
    currentEdge[1] = 7;
    discrepancyInLocation = false;    
  }

  else if(leftDiff > LONGENCMINVAL || rightDiff > LONGENCMINVAL){ //using left diff because that will be largest if we default to turning right if not straight
    motor.stop_all(); LCD.clear(); LCD.print("On 17-18"); delay(200);
    if(rightTurnPossible >= PATHCONFIDENCE && leftTurnPossible < PATHCONFIDENCE){
      discrepancyInLocation = false;
      if(hasPassenger){
        turnCCW();
        passengerSpotted = 0;
        currentEdge[0] = 17;
        currentEdge[1] = 18;
      }
      else{
        currentEdge[0] = 17;
        currentEdge[1] = 16;
      }
      profitMatrix[nodeMat[17][18]][17] = 0;
      profitMatrix[nodeMat[18][17]][18] = 0;
    }
    else if(leftTurnPossible >= PATHCONFIDENCE && rightTurnPossible < PATHCONFIDENCE){
      discrepancyInLocation = false;
      if(hasPassenger){
        turnCW();
        passengerSpotted = 0;
        currentEdge[0] = 18;
        currentEdge[1] = 17;
      }
      else{
        currentEdge[0] = 18;
        currentEdge[1] = 19;
      }
      profitMatrix[nodeMat[17][18]][17] = 0;
      profitMatrix[nodeMat[18][17]][18] = 0;
    }
    //reset incircle for next time we are lost
    if(!discrepancyInLocation){
      //executes once when we find ourselves
      LCD.clear(); LCD.print("WE MADE IT!!"); delay(200);
      inCircle = false;
    }
  }
}

