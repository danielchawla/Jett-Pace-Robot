#define stage1 15 // 10
#define stage2 90 // maybe 95 and stage1 15 is better
#define stage3 70 // Doesnt matter what this is. <--add 0's to make a three stage u-turn
#define tooManyRevs 500

int offTape = false;
int outOfCollision = false;
int lastEncCount = 0;
int loopsSinceLastChange = 0;
int stuck = false;

void turnAround(int reverseMotor, int driveMotor, volatile unsigned int &reverseEncoderCount, volatile unsigned int &driveEncoderCount){
	loopsSinceLastInt = 200;
	//Stage1: Reverse just left
	int stage = 0;
	int tempInt180;
	int buttonIn180Confidence = 0;

	while(true){
		// While loop is set up with if statements for different stages similar to turning code at intersections
		if(stage == 0){
			stage++;
			motor.stop_all();
			LCD.clear();LCD.print("About to Turn");
			LCD.clear(); LCD.print("Stage 1");
			count180 = reverseEncoderCount;
			motor.speed(reverseMotor, -1*MAX_MOTOR_SPEED*2/3);
		}

		if(stage == 1 && reverseEncoderCount - count180 > stage1){ // This is the condition to leave stage 1 - at this point we write Stage 2 speeds and increment stage
			stage++;
			motor.stop_all();
			//Entering Stage 2: Reverse both
			LCD.clear();LCD.print("Stage 2 "); LCD.print(stuck);
			count180 = reverseEncoderCount;
			motor.speed(reverseMotor, -1*MAX_MOTOR_SPEED*2/3);
			motor.speed(driveMotor, -1*MAX_MOTOR_SPEED/7);
		}

		if(stage == 2 && (reverseEncoderCount - count180 > stage2 || (stuck && reverseEncoderCount - count180 > stage2/4))) { 
			stage++;
			motor.stop_all();
			// Entering Stage 3: Pivot
			LCD.clear();LCD.print("Stage 3 "); LCD.print(stuck);
			count180 = driveEncoderCount;
			motor.speed(reverseMotor, -1*MAX_MOTOR_SPEED/4);
			motor.speed(driveMotor, MAX_MOTOR_SPEED*2/3);
			stuck = false;
		}
		if (digitalRead(FRONT_BUMPER_PIN) || digitalRead(FRONT_LEFT_BUMPER_PIN) || digitalRead(FRONT_RIGHT_BUMPER_PIN)){
			buttonIn180Confidence++;
		}
		else if(buttonIn180Confidence > 0){
			buttonIn180Confidence -= 2;
		}
		if(stage == 3 && (driveEncoderCount - count180 > tooManyRevs || buttonIn180Confidence >= 15)) {
			//set to stage 2:
			stage = 1;
			count180 = -1*stage1; //gonna go into that if statement up there once and will leave in stage 3
			loopsSinceLastChange = 0;
			buttonIn180Confidence = 0;
			stuck = true;
		}


		//Check if offTape
		if(!offTape){
			if(stage > 1 && !digitalRead(q1) && !digitalRead(q2)) {
				statusCount180++;
				if(statusCount180 > 200){
					//we just lost the tape.
					statusCount180 = 0;
					offTape = true;
				}
			}
			else if(statusCount180){
				statusCount180--;
			}
		}
		//Check if tape is found
		if(offTape){
			if(digitalRead(q1) || digitalRead(q2)){// Need to change the stage > condition - not adaptable enough, although maybe is very useful information on how far we've turned
				statusCount180++;

				if(statusCount180 > 10){
					//Finished turning around - change currentEdge
					motor.stop_all();
					statusCount180 = 0;
					stage = 0; // Reset stage
					offTape = false;
					leftEncoderAtLastInt = leftCount;
					rightEncoderAtLastInt = rightCount;
					if(discrepancyInLocation){
						motor.speed(BUZZER_PIN, MAX_MOTOR_SPEED/5);
					}
					break;
				}
			}
			else if(statusCount180){
				statusCount180--;
			}
		}

		if(loopsSinceLastChange > 20000){
			LCD.clear(); LCD.print("STUCK");
			motor.stop_all();
			//do something with regards wto changing stage back to one that would be appropriate.
			if(stage != 3){
				//set to stage 3:
				stage = 2;
				count180 = -1*stage2; //gonna go into that if statement up there once and will leave in stage 3
				loopsSinceLastChange = 0;
			}
			else{
				//set to stage 2:
				stage = 1;
				count180 = -1*stage1; //gonna go into that if statement up there once and will leave in stage 2
				loopsSinceLastChange = 0;
				stuck = true;
			}
		}
		else{
			if(stage == 3){
				//use driveEncoderCount
				if(lastEncCount == driveEncoderCount){
					loopsSinceLastChange++;
				}
				else{
					loopsSinceLastChange = 0;
					lastEncCount = driveEncoderCount;
				}
			}
			else{
				//use reverseEncoderCount
				if(lastEncCount == reverseEncoderCount){
					loopsSinceLastChange++;
				}
				else{
					loopsSinceLastChange = 0;
					lastEncCount = reverseEncoderCount;
				}
			}
		}

	}
}
	
void turn180Decision(){
	rightDiff = rightCount - rightEncoderAtLastInt;
  leftDiff = leftCount - leftEncoderAtLastInt;
	loopsSinceLastInt = 0;
  LCD.clear(); LCD.print("L: "); LCD.print(rightDiff); LCD.print(" R: "); LCD.print(leftDiff);
  int turnDirection = pastTurn;
  int distFromIntCase = GARBAGE;
  if(atIntersection){
  	if(turnActual == STRAIGHT || turnActual == GARBAGE){
  		switch (pastTurn){
  			case RIGHT: turnCW(); break;
  			case LEFT: turnCCW(); break;
  			default: turnCCW(); motor.stop_all(); LCD.clear(); LCD.print("past turn straight"); /*while(true){}*/ break;
  		}
  	}else if(turnActual == LEFT){
  		turnCCW();
  	}else if(turnActual == RIGHT){
  		turnCW();
  	}
  	resetIntersection();
  	atIntersection = 0;
  	discrepancyInLocation = true;
  }else{ // if !atIntersection
  	switch(currentEdge[1]){
  		case 2: distFromIntCase = 1; break; // Close to Int condition at these nodes
  		case 1: distFromIntCase = 2; break;
  		case 3: distFromIntCase = 2; break; // Far from Int condition at thses nodes
  	}
  	if(distFromIntCase == 1 || (leftDiff < CLOSETOINTCOUNT && rightDiff < CLOSETOINTCOUNT && !discrepancyInLocation)){
  		switch(currentEdge[1]){
  			case 5: turnCCW(); turnDirection = LEFT; break;
  			case 9: turnCW(); turnDirection = RIGHT; break;
  			case 2:
  				if(pastTurn == LEFT){
  					turnCW();
  					turnDirection = RIGHT;
  					currentEdge[0] = 7;
  					currentEdge[1] = 13;
  				}
  				else{ //if past turn is right
  					turnCCW();
  					turnDirection = LEFT;
  					currentEdge[0] = 7;
  					currentEdge[1] = 12;  					
  				} break;
  			default:
		  		if(pastTurn == LEFT){
		  			turnCCW();
		  		}else{
		  			turnCW();
		  		}
		  }
		  if(currentEdge[0] != 7){
	  		for(int i = 1; i < 4; i++){
	  			if(theMap[nodeMat[currentEdge[0]][currentEdge[1]]+i*turnDirection][currentEdge[0]] != -1){
	  				currentEdge[1] = theMap[nodeMat[currentEdge[0]][currentEdge[1]]+i*turnDirection][currentEdge[0]];
	  				break;
	  			}
	  			if(i == 3){
	  				motor.stop_all(); LCD.clear(); LCD.print("bug #88Ø"); delay(2000);
	  			}
	  		}
	  	}
  	}else if(distFromIntCase == 2 || (leftDiff > FARFROMINTCOUNT && rightDiff > FARFROMINTCOUNT && !discrepancyInLocation)){
			switch(currentEdge[1]){
  			case 1: turnCCW(); tapeFollowVel = vel*1/2; slowedDown = true; break;
  			case 3: turnCW(); tapeFollowVel = vel*1/2; slowedDown = true; break;
  			default:
		  		if(pastTurn == LEFT){
		  			turnCCW();
		  		}else{
		  			turnCW();
		  		}
		  }  		
			int tempInt180 = currentEdge[1];
			currentEdge[1] = currentEdge[0];
			currentEdge[0] = tempInt180;
  	} else{ //grey zone or already lost
  		if(!discrepancyInLocation){
  			motor.stop_all(); LCD.clear(); LCD.print("In grey zone"); delay(1000);
  		}
			switch(currentEdge[1]){
  			case 5: turnCCW(); turnDirection = LEFT; break;
  			case 9: turnCW(); turnDirection = RIGHT; break;
  			default:
		  		if(pastTurn == LEFT){
		  			turnCCW();
		  		}else{
		  			turnCW();
		  		}
		  }
  		if(currentEdge[1] == 5 || currentEdge[1] == 9){
  			int tempInt180 = currentEdge[1];
				currentEdge[1] = currentEdge[0];
				currentEdge[0] = tempInt180;
				desiredTurn = pastTurn;
    	}else{
    		currentEdge[0] = GARBAGE;
  			currentEdge[1] = GARBAGE;
  			leftInitial = GARBAGE;
      	rightInitial = GARBAGE;
      	if(discrepancyInLocation){
      		desiredTurn = STRAIGHT;
      	}else{
      		desiredTurn = -1*pastTurn;
      	}
      	discrepancyInLocation = true;
    	}
  		loopsSinceLastInt = 1000;
  	}
  }
  pastTurn = pastTurn*-1; //Switch pastDirection so that next turn is in proper direction
	leftEncoderAtLastInt = leftCount;
	rightEncoderAtLastInt = rightCount;
	currentDir = (nodeMat[currentEdge[1]][currentEdge[0]] + 2) % 4;//direction with which we will enter the next intersection.
}

void turnCCW(){
	turnAround(LEFT_MOTOR, RIGHT_MOTOR, leftCount, rightCount);	// Set currentEdge for special cases - These currentEdges are the already flipped ones as if the turn had been completed on a single edge successfull
}

void turnCW(){
	turnAround(RIGHT_MOTOR, LEFT_MOTOR, rightCount, leftCount);
}

void collisionCheck(){
	if((digitalRead(OR) || digitalRead(FRONT_LEFT_BUMPER_PIN)) && !collisionDetected && !passengerSide){
    collisionCount++;
    switchVals[FRONT_BUMPER] += digitalRead(FRONT_BUMPER_PIN);
    switchVals[FRONT_RIGHT_BUMPER] += digitalRead(FRONT_RIGHT_BUMPER_PIN);
    switchVals[RIGHT_BUMPER] += digitalRead(RIGHT_BUMPER_PIN);
    switchVals[REAR_BUMPER] += digitalRead(REAR_BUMPER_PIN);
    switchVals[LEFT_BUMPER] += digitalRead(LEFT_BUMPER_PIN);
    switchVals[FRONT_LEFT_BUMPER] += digitalRead(FRONT_LEFT_BUMPER_PIN);
    if(collisionCount > 10){
    	switchVals[FRONT_BUMPER] /= 5;
      switchVals[FRONT_RIGHT_BUMPER] /= 5;
      switchVals[RIGHT_BUMPER] /= 5;
      switchVals[REAR_BUMPER] /= 5;
      switchVals[LEFT_BUMPER] /= 5;
      switchVals[FRONT_LEFT_BUMPER] /= 5;
      collisionDetected = true;
      collisionCount = 0;

    }
  } else if(collisionCount){
    collisionCount--;
  }
}

void ReverseSlow(int fastMotor, int slowMotor, volatile unsigned int &encoderCount){
	unsigned long reverseStart = millis();
	int initialEncoderCount = encoderCount;
	while(encoderCount - initialEncoderCount < 100){
		motor.speed(fastMotor, -MAX_MOTOR_SPEED*1/2);
		motor.speed(slowMotor, -MAX_MOTOR_SPEED*1/3);
		if(digitalRead(q1) || digitalRead(q2) || millis() - reverseStart > 1000){
			break;
		}
	}
}

void reverseRight(){
	ReverseSlow(LEFT_MOTOR, RIGHT_MOTOR, leftCount);
}
void reverseLeft(){
	ReverseSlow(RIGHT_MOTOR, LEFT_MOTOR, rightCount);
}