void CheckForPassenger() {
  // Check left side
  if (leftIRVal > leftIRValMax) {
    leftIRValMax = leftIRVal;
  } else if (leftIRVal < leftIRValMax - 10) {
    // Stop motors and pick up passenger
    motor.speed(0, -255);
    motor.speed(0, -255);
    delay(100);
    motor.speed(0, 0);
    motor.speed(0, 0);
    hasPassenger = PickupPassenger(1);
    leftIRValMax = -1;
  }
  // Check right side
  if (rightIRVal > rightIRValMax) {
    // Stop motors and pick up passenger
    rightIRValMax = rightIRVal;
  } else if (rightIRVal < rightIRValMax - 10) {
    motor.speed(0, -255);
    motor.speed(0, -255);
    delay(100);
    motor.speed(0, 0);
    motor.speed(0, 0);
    hasPassenger = PickupPassenger(-1);
    rightIRValMax = -1;
  }
}

int PickupPassenger(int side) { // side=1 if on left, side=-1 if on right
  int range = 80;
  int tripThresh = 800;
  int armDelay = 15;
  int maxIR = -1;
  int newIR = -1;
  int finalI = range - 1;

  RCServo0.write(clawHome);
  RCServo1.write(armHome);

  LCD.clear(); LCD.print("Picking Up");

  // Position Arm
  for (int i = 0; i <= range; i++) {
    RCServo1.write(armHome + i * side);
    delay(armDelay);
    newIR = analogRead(ArmIRpin);
    if (newIR > maxIR) {
      maxIR = newIR;
    } else if (newIR < maxIR - 10) {
      finalI = i;
      break;
    }
    LCD.clear(); LCD.print("IR:  "); LCD.print(newIR);
    LCD.setCursor(0, 1); LCD.print("Max: "); LCD.print(maxIR);
  }

  // Position Claw
  motor.speed(2, 150);
  for (int i = 0; i < 2000; i++) { // Will need to change this
    if (!digitalRead(clawTrip)) {
      break;
    }
    delay(1);
  }
  motor.speed(2, 0);

  // Close Claw
  RCServo0.write(clawClose);
  delay(1000);

  // Retract Claw
  motor.speed(2, -150);
  delay(400);
  // Home Arm
  for (int i = 0; i <= finalI; i++) {
    RCServo1.write(armHome + (finalI - i)*side);
    delay(armDelay);
  }

  if (1600 - range * armDelay > 0) {
    delay(1600 - range * armDelay);
  }
  motor.speed(2, 0);

  // Home Claw
  RCServo0.write(clawHome);
  delay(1000);

  return 1;
}

