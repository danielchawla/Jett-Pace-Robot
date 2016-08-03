int CheckForPassenger() { 
  leftIRVal = analogRead(leftIR);
  rightIRVal = analogRead(rightIR);
  // Check left side
  // if((leftIRVal > sideIRMin || rightIRVal > sideIRMin) && !hasPassenger){ // this didn't work
  //   tapeFollowVel = vel * 0.6;
  // } else if(tapeFollowVel != vel){
  //   tapeFollowVel = vel;
  // }
  //   leftIRValMax = leftIRVal;
  // } else if (leftIRVal < leftIRValMax - 15 && leftIRValMax > sideIRMin) { // TODO: check this  
  if (leftIRVal > sideIRMin){ // not searching for max anymore. 
  // Stop motors, reset maxima and pick up passenger
  passengerSeenCount++;
    if(passengerSeenCount > 10){
      leftIRValMax = -1;
      rightIRValMax = -1;
      return LEFT;
    }
  }
  // Check right side
  //   rightIRValMax = rightIRVal;
  // } else if (rightIRVal < rightIRValMax - 15 && rightIRValMax > sideIRMin) { // TODO: check this
  else if (rightIRVal > sideIRMin){
    // Stop motors, reset maxima and pick up passenger
    passengerSeenCount++;
    if(passengerSeenCount > 10){
      motor.stop_all();
      leftIRValMax = -1;
      rightIRValMax = -1;
      return RIGHT;
    }
  } else if(passengerSeenCount){
    passengerSeenCount--;
  }
  return 0;
}

int PickupPassenger(int side) { // side=-1 if on left, side=1 if on right
  int range = 70;
  int armDelay = 11;
  int maxIR = -1;
  int newIR = -1;
  int finalI = range - 1;

  RCServo0.write(clawMid);
  RCServo1.write(armHome);

  LCD.clear(); LCD.print("Picking Up");

  // Rotate arm until max is spotted
  startTime = millis();
  for (int i = 0; i <= range; i++) {
    RCServo1.write(armHome + i * side);

    while(millis() - startTime < armDelay){}
    startTime = millis();

    newIR = analogRead(ArmIRpin);
    if (newIR > maxIR) {
      maxIR = newIR;
    } else if (newIR < maxIR - 20 && maxIR > armIRMin) {
      // LCD.clear();
      // LCD.print("Found Max"); LCD.setCursor(0,1);LCD.print(maxIR);
      // motor.speed(BUZZER_PIN, MAX_MOTOR_SPEED*3/4);
      // delay(1000);
      // motor.speed(BUZZER_PIN, 0);
      // RCServo1.write(armHome + (i-2)*side);
      // finalI = i;
      // break;
    }
    LCD.clear(); LCD.print("IR:  "); LCD.print(newIR);
    LCD.setCursor(0, 1); LCD.print("Max: "); LCD.print(maxIR);
  }


  LCD.clear(); LCD.print("Closing Claw");
  // Extend claw
  motor.speed(GM7, -150);
  startTime = millis();
  while(millis() - startTime < 1200){}
  motor.speed(GM7, 0);

  // Close claw
  RCServo0.write(clawClose);
  startTime = millis();
  while(millis() - startTime < 800){}
  
  motor.speed(GM7, 150);
  startTime = millis();
  while(millis() - startTime < 600){}
  // Rotate claw back to center
  RCServo1.write(armHome);
  /*startTime = millis();
  for (int i = 0; i <= finalI; i++) {
    RCServo1.write(armHome + (finalI - i)*side);
    while(millis() - startTime < armDelay){}
    startTime = millis();
  }*/
  startTime = millis();
  while(millis() - startTime < 1200){}
  motor.speed(GM7, 0);

  // Checks if side pickup attempt was successful
  if((side == LEFT && analogRead(leftIR) >= SIDEPICKUPSUCCESSTHRESH) || (side == RIGHT && analogRead(rightIR) >= SIDEPICKUPSUCCESSTHRESH)){ // TODO: maybe use a passengerGoneThresh if this doesn't work
    LCD.clear(); LCD.print("No Passenger"); delay(500);
    return 0;
  } // Checks if front pickup attempt was successful
  else if(side == STRAIGHT && analogRead(ArmIRpin) >= FRONTPICKUPSUCCESSTHRESH){
    LCD.clear(); LCD.print("No Passenger"); delay(500);
    return 0;
  }
  return 1;
}

/*void DistanceDetectPassenger(){
  if(digitalRead(leftDistanceIR) > passengerInDistanceThreshold && analogRead(rightIRDistance) < passengerInDistanceThreshold){
    desiredTurn = LEFT;
  } else if(digitalRead(leftDistanceIR) < passengerInDistanceThreshold && analogRead(rightIRDistance) > passengerInDistanceThreshold)){
    desiredTurn = RIGHT;
  }
}*/

void DropoffPassenger(int side){
  LCD.clear(); LCD.print("Dropping off");
  delay(1000);
  int range = 80;
  int armDelay = 15;
  int startTime;
  
  // Extend claw
  motor.speed(GM7, -150);
  delay(1000);
  //startTime = millis();
  //while(millis() - startTime < 1000){}
  motor.speed(GM7, 0);

  // Rotate Arm
  RCServo1.write(armHome + range * side);
  delay(800); // was 300. changed so it rotates all the way before letting go
  // startTime = millis();
  // while(millis() - startTime < 300){}

  // Open Claw
  RCServo0.write(clawOpen);
  delay(600);
  // startTime = millis();
  // while(millis() - startTime < 600){}

  // Close claw
  RCServo0.write(clawClose);
  
  // Retract claw
  motor.speed(GM7, 150);
  delay(300);
  // startTime = millis();
  // while(millis() - startTime < 300){}

  // Rotate arm back to center
  RCServo1.write(armHome);
  delay(1500);
  // startTime = millis();
  // while(millis() - startTime < 1500){}
  motor.speed(GM7, 0);

  hasPassenger = false;
}