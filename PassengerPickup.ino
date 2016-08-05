int checkForPassenger() { 
  leftIRVal = analogRead(LEFTIR);
  rightIRVal = analogRead(RIGHTIR);
  // Check left side
  if (leftIRVal > SIDEIRMIN){ // not searching for max anymore. 
  // Stop motors, reset maxima and pick up passenger
  passengerSeenCount++;
    if(passengerSeenCount > 10){
      leftIRValMax = -1;
      rightIRValMax = -1;
      return LEFT;
    }
  }
  // Check right side
  else if (rightIRVal > SIDEIRMIN){
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

int pickupPassenger(int side) { // side=-1 if on left, side=1 if on right
  int range = 70;
  int armDelay = 11;
  int maxIR = -1;
  int newIR = -1;
  int finalI = range - 1;
  if(currentEdge[1] == 5 && side == LEFT){
    range = 66;
  }else if((currentEdge[1] == 12 && currentEdge[0] == 13) || (currentEdge[0] == 12 && currentEdge[1] == 13)){
    LCD.clear();motor.stop_all();LCD.print("Setting Range"); delay(1000);
    range = 80;
  }
  RCServo0.write(CLAWMID);
  RCServo1.write(ARMHOME);

  LCD.clear(); LCD.print("Picking Up");

  RCServo1.write(ARMHOME + side * range);
  delay(600);

  LCD.clear(); LCD.print("Closing Claw");
  // Extend claw
  motor.speed(GM7, -150);
  startTime = millis();
  while(millis() - startTime < 1200){}
  motor.speed(GM7, 0);

  // Close claw
  RCServo0.write(CLAWCLOSE);
  startTime = millis();
  while(millis() - startTime < 800){}
  
  motor.speed(GM7, 150);
  startTime = millis();
  while(millis() - startTime < 600){}
  // Rotate claw back to center
  RCServo1.write(ARMHOME);
  startTime = millis();
  while(millis() - startTime < 1200){}
  motor.speed(GM7, 0);

  // Checks if side pickup attempt was successful
  if((side == LEFT && analogRead(LEFTIR) >= SIDEPICKUPSUCCESSTHRESH) || (side == RIGHT && analogRead(RIGHTIR) >= SIDEPICKUPSUCCESSTHRESH)){ 
    delay(1);
    if((side == LEFT && analogRead(LEFTIR) >= SIDEPICKUPSUCCESSTHRESH) || (side == RIGHT && analogRead(RIGHTIR) >= SIDEPICKUPSUCCESSTHRESH)){
      delay(1);
      if((side == LEFT && analogRead(LEFTIR) >= SIDEPICKUPSUCCESSTHRESH) || (side == RIGHT && analogRead(RIGHTIR) >= SIDEPICKUPSUCCESSTHRESH)){
        LCD.clear(); LCD.print("No Passenger"); delay(500);
        return 0;
      }
    }
  } // Checks if front pickup attempt was successful
  else if(side == STRAIGHT && analogRead(ARMIRPIN) >= FRONTPICKUPSUCCESSTHRESH){
    LCD.clear(); LCD.print("No Passenger"); delay(500);
    return 0;
  }
  return 1;
}

void dropoffPassenger(int side){
  LCD.clear(); LCD.print("Dropping off");
  //delay(1000);
  int range = 80;
  int armDelay = 15;
  int startTime;
  
  // Extend claw
  motor.speed(GM7, -150);
  delay(1000);

  motor.speed(GM7, 0);

  // Rotate Arm
  RCServo1.write(ARMHOME + range * side);
  delay(800); // was 300. changed so it rotates all the way before letting go

  // Open Claw
  RCServo0.write(CLAWOPEN);
  delay(600);

  // Close claw
  RCServo0.write(CLAWCLOSE);
  
  // Retract claw
  motor.speed(GM7, 150);
  delay(300);

  // Rotate arm back to center
  RCServo1.write(ARMHOME);
  delay(1500);
  motor.speed(GM7, 0);

  hasPassenger = false;
}