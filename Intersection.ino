void AreWeThereYet(){
  if ((qrdVals[0] == HIGH || qrdVals[3] == HIGH) && (qrdVals[1] == HIGH || qrdVals[2] == HIGH) && loopsSinceLastInt > 1000) {
    //statusCount++;
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

  }/* else if (statusCount) {
    statusCount--;
  }*/
  if (leftTurnPossible > pathConfidence || rightTurnPossible > pathConfidence) { // Makes harder to see intersection
    motor.speed(LEFT_MOTOR, -1 * MAX_MOTOR_SPEED);
    motor.speed(RIGHT_MOTOR, -1 * MAX_MOTOR_SPEED);
    motor.stop_all();
    atIntersection = 1;
    statusCount = 0;
  }
  if (atIntersection == 1) {
    LCD.clear();
    LCD.print("Going Straight");

    turn180 = (qrdVals[0] == LOW && qrdVals[1] == LOW && qrdVals[2] == LOW && qrdVals[3] == LOW);

    // TODO: used for check to see if we know where we are function. TODOLOST
     rightDiff = rightCount - rightEncoderAtLastInt;
     leftDiff = leftCount - leftEncoderAtLastInt;
  }
}


void ProcessIntersection() {
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

    /*if (countInIntersection > maxInIntersection) {
      atIntersection = 0;
      LCD.clear();
      LCD.print("Leaving");
    }*/

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

      motor.speed(LEFT_MOTOR, vel / 4 - avgCorrection);
      motor.speed(RIGHT_MOTOR, vel / 4 + avgCorrection); // TODO: CHANGE may need to have to set back to /4
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

    if (!qrdVals[0] && !qrdVals[3] && !tapeFollowCountInInt) {
      leavingCount++;
      LCD.clear(); LCD.print("STRAIGHT");
      if(leavingCount > 50){ //may need to CHANGE for time trials 200 -> 10.  May try to go straight when not possible though
        tapeFollowCountInInt = 1;
        turnActual = STRAIGHT;
        //atIntersection = 0;
      }
      /*if(leavingCount > 200){
        atIntersection = 0;
      }*/
    }

    if(tapeFollowCountInInt){
      tapeFollowCountInInt++;
      TapeFollow();
      if(tapeFollowCountInInt > 600){ // previously 1000

        // motor.stop_all();
        // LCD.clear();
        // LCD.print(noStraightCount);
        // delay(1200);
        //atIntersection = false; //jonah had this. TODO: remove this if ryan's shit below doesnt work
        //Ryans stuff begins
        //we can't go straight.
        // motor.stop_all();
        // delay(500); // TODO get rid of this and stop all
        
        // need to turn
        if(leftTurnPossible>=pathConfidence){
          turnActual = LEFT;
          turning = 1;
          qrdToCheck = q0;
          loopNum = 2;
          LCD.clear();
          LCD.print("Turning Left");
        } else if(rightTurnPossible>=pathConfidence){
          turnActual = RIGHT;
          turning = 1;
          qrdToCheck = q3;
          loopNum = 2;
          LCD.clear();
          LCD.print("Turning Right");
        } else{ // THIS SHOULD NEVER HAPPEN
          LCD.print("No straight or turn");
          motor.stop_all();
          while(true){} // TODO 
        }
        //ryans stuff ends
      }
    }

    // Check if all QRDs are lost
    /*if((qrdVals[0] == LOW && qrdVals[1] == LOW && qrdVals[2] == LOW && qrdVals[3] == LOW) && tapeFollowCountInInt){
      noStraightCount++;
      if(noStraightCount > 1200){
        motor.stop_all();
        delay(2000); // TODO get rid of this and stop all
        // need to turn
        if(leftTurnPossible>=pathConfidence){
          turnActual = LEFT;
          turning = 1;
          qrdToCheck = q0;
          loopNum = 2;
          LCD.clear();
          LCD.print("Turning Left");
        } else if(rightTurnPossible>=pathConfidence){
          turnActual = RIGHT;
          turning = 1;
          qrdToCheck = q3;
          loopNum = 2;
          LCD.clear();
          LCD.print("Turning Right");
        } else{ // THIS SHOULD NEVER HAPPEN
          LCD.print("No straight or turn");
          motor.stop_all();
          while(true){} // TODO 
        }
      }
    }else if((qrdVals[0] == HIGH || qrdVals[1] == HIGH || qrdVals[2] == HIGH || qrdVals[3] == HIGH) && noStraightCount>0){
      noStraightCount-=100;
    }*/

      //RYANS NEW CODE
      //check if there is tape after int
    //if((digitalRead(q0) || digitalRead(q1) || digitalRead(q3) || digitalRead(q2)) && tapeFollowCountInInt > 300){
    if((qrdVals[0] == HIGH || qrdVals[1] == HIGH || qrdVals[2] == HIGH || qrdVals[3] == HIGH) && tapeFollowCountInInt > 300){
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
      if(leftTurnPossible > pathConfidence) {
        turnActual = LEFT;
        turning = 1;
        qrdToCheck = q0;
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
    // TODO copy paste lost detection code from DEV - Is below - Uncomment and make sure it works

    /*
      Check if we could not turn the way we wanted to - if we couldn't and decision making is working correctly, then we're lost
      Could be updated to be a bit smarter, and use what was actually seen if we went straight to further determine if we're lost
      Currently, we can always go straight even at an L or T intersection, so this is not 100% reliable but will (should) never give false positives
    */
      //TODOLOST - uncomment and see if works
    /*
    rightEncoderAtLastInt = rightCount;
    leftEncoderAtLastInt = leftCount;
    if(desiredTurn != turnActual){
      discrepancyInLocation = true;
    }
    if(leftTurnPossible >= pathConfidence){
      if(theMap[(currentDir + LEFT + 4) % 4][currentEdge[1]] == -1){
        discrepancyInLocation = true;
      }
    }
    if(rightTurnPossible >= pathConfidence){
      if(theMap[(currentDir + RIGHT + 4) % 4][currentEdge[1]] == -1){
        discrepancyInLocation = true;
      }
    }
    if(turnActual == STRAIGHT){
      if(leftTurnPossible <pathConfidence){
        if(theMap[(currentDir + LEFT + 4) % 4][currentEdge[1]] != -1){
          discrepancyInLocation = true;
        }
      }
      if(rightTurnPossible < pathConfidence){
        if(theMap[(currentDir + RIGHT + 4) % 4][currentEdge[1]] != -1){
          discrepancyInLocation = true;
        }
      }
    }*/

    //TODOLOST - uncomment
    if(discrepancyInLocation){
      motor.speed(BUZZER_PIN, MAX_MOTOR_SPEED/5);
      checkToSeeIfWeKnowWhereWeAre(); //this is called right after an intersection
    }

    motor.speed(BUZZER_PIN, 0);
    if(desiredTurn != turnActual){
      discrepancyInLocation = true;
    }

    // Update the current edge based on turnActual
    if(!discrepancyInLocation){
      currentEdge[0] = currentEdge[1];
      currentEdge[1] = theMap[(currentDir + turnActual + 4) % 4][currentEdge[0]];
    }

    desiredTurn = GARBAGE;
    turnActual = GARBAGE;

    countInIntersection = 0;
    turning = 0;
    leftTurnPossible = 0;
    rightTurnPossible = 0;
    leavingCount = 0;

    loopsSinceLastInt = 0;
    tapeFollowVel = vel;
    loopNum = 1;
    lostTape = 0;
    noStraightCount = 0;
    tapeFollowCountInInt = 0;
    avgCorrection = 0;
  }
}

void checkToSeeIfWeKnowWhereWeAre(void){
  //Checks to see if we are going along the straight edge by the drop off.
  
  if(turnActual == STRAIGHT){
    numOfCOnsecutiveStraights++;

    if(numOfCOnsecutiveStraights == 2){
      //we can check differences in encoders to determine where we are.
      if(leftDiff - rightDiff > diffInCircle || rightDiff - leftDiff > diffInCircle){
        inCircle = true;
      }
    }
    /*if(numOfCOnsecutiveStraights == 3){
      inCircle = true;
    }*/
  }
  else{
    numOfCOnsecutiveStraights = 0;
    inCircle = false;
  }

  if(inCircle){
    if(leftDiff > curveOutsideCount || rightDiff > curveOutsideCount){
      if(rightTurnPossible >= pathConfidence && leftTurnPossible < pathConfidence){
        currentEdge[0] = 13;
        currentEdge[1] = 7;
        discrepancyInLocation = false;
      }
      else if(leftTurnPossible >= pathConfidence && rightTurnPossible < pathConfidence){
        currentEdge[0] = 12;
        currentEdge[1] = 7;
        discrepancyInLocation = false;
      }
      else{
        discrepancyInLocation = true;
      }
    }
  }
  else if(leftDiff > leftEncMinVal){ //using left diff because that will be largest if we default to turning right if not straight
    if(rightTurnPossible >= pathConfidence && leftTurnPossible < pathConfidence){
      currentEdge[0] = 17;
      currentEdge[1] = 16;
      discrepancyInLocation = false;
    }
    else if(leftTurnPossible >= pathConfidence && rightTurnPossible < pathConfidence){
      currentEdge[0] = 18;
      currentEdge[1] = 19;
      discrepancyInLocation = false;
    }
    else{
      discrepancyInLocation = true;
    }
  }

  //reset incircle for next time we are lost
  if(!discrepancyInLocation){
    inCircle = false;
  }
}

/*
bool sortaEqual(int a, int b){
  int diff = a-b;
  if (diff < 0){
    diff = -1*diff;
  }
  if(diff < 30){
    return true;
  }
  return false;
}
*/
