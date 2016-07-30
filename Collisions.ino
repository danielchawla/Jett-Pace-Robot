#define stage1 15 // 10
#define stage2 90 // maybe 95 and stage1 15 is better
#define stage3 90

int offTape = false;
int outOfCollision = false;
int lastEncCount = 0;
int loopsSinceLastChange = 0;

void TurnAround(int reverseMotor, int driveMotor, volatile unsigned int &reverseEncoderCount, volatile unsigned int &driveEncoderCount){
	//Stage1: Reverse just left
	int stage = 0;
	int tempInt180;

	while(true){
		// While loop is set up with if statements for different stages similar to turning code at intersections
		if(stage == 0){
			stage++;
			motor.stop_all();
			LCD.clear();LCD.print("About to Turn");
			// delay(100);
			LCD.clear(); LCD.print("Stage 1");
			count180 = reverseEncoderCount;
			motor.speed(reverseMotor, -1*MAX_MOTOR_SPEED*2/3);
		}

		if(stage == 1 && reverseEncoderCount - count180 > stage1){ // This is the condition to leave stage 1 - at this point we write Stage 2 speeds and increment stage
			stage++;
			motor.stop_all();
			// delay(100);
			//Entering Stage 2: Reverse both
			LCD.clear();LCD.print("Stage 2 "); LCD.print(reverseEncoderCount - count180);
			count180 = reverseEncoderCount;
			motor.speed(reverseMotor, -1*MAX_MOTOR_SPEED*2/3);
			motor.speed(driveMotor, -1*MAX_MOTOR_SPEED/7);
		}

		if(stage == 2 && reverseEncoderCount - count180 > stage2){ // should maybe change digital read to be more robust
			stage++;
			motor.stop_all();
			// delay(100);
			// Entering Stage 3: Pivot
			LCD.clear();LCD.print("Stage 3 "); LCD.print(driveEncoderCount - count180);
			count180 = driveEncoderCount;
			motor.speed(reverseMotor, -1*MAX_MOTOR_SPEED/4);
			motor.speed(driveMotor, MAX_MOTOR_SPEED*2/3);
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
					statusCount180 = 0;
					stage = 0; // Reset stage
					offTape = false;
					loopsSinceLastInt = 500; // TODO: investigate
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

		if(loopsSinceLastChange > 20000){ //Previously was 40000
			LCD.clear(); LCD.print("STUCK");
			motor.stop_all();
			//do something with regards to changing stage back to one that would be appropriate.
			if(stage < 3){
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
				//use leftEncoderCount
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
	
void Turn180Decision(){
	rightDiff = rightCount - rightEncoderAtLastInt;
  leftDiff = leftCount - leftEncoderAtLastInt;

  LCD.clear(); LCD.print("L: "); LCD.print(rightDiff); LCD.print(" R: "); LCD.print(leftDiff);
  motor.stop_all(); delay(3000);
  int turnDirection = pastTurn;
  int distFromIntCase = GARBAGE;
  if(atIntersection){
  	if(turnActual == STRAIGHT){
  		switch (pastTurn){
  			case RIGHT: TurnCCW(); break;
  			case LEFT: TurnCW(); break;
  			default: TurnCCW(); motor.stop_all(); LCD.clear(); LCD.print("past turn straight"); while(true){} break;
  		}
  	}
  	ResetIntersection();
  	motor.stop_all(); LCD.clear(); LCD.print("At Intersection ??"); delay(1000);
  	discrepancyInLocation = true;
  }else{ // if !atIntersection
  	switch(currentEdge[1]){
  		case 5: distFromIntCase = 1; break; 
  		case 9: distFromIntCase = 1; break;
  		case 2: distFromIntCase = 1; break; // Close to Int condition at these nodes
  		case 1: distFromIntCase = 2; break;
  		case 3: distFromIntCase = 2; break; // Far from Int condition at thses nodes
  	}
  	if(distFromIntCase == 1 || (leftDiff < closeToIntCount && rightDiff < closeToIntCount && !discrepancyInLocation)){// TODO see whether && || || is apropriate
  		switch(currentEdge[1]){
  			case 5: TurnCCW(); turnDirection = LEFT; break;
  			case 9: TurnCW(); turnDirection = RIGHT; break;
  			case 2:
  				if(pastTurn == LEFT){
  					TurnCW(); // TODO switch turnDirection??
  					turnDirection = RIGHT;
  				}
  				else{ //if past turn is right
  					TurnCCW();
  					turnDirection = LEFT;
  				}
  			default:
		  		if(pastTurn == LEFT){
		  			TurnCCW();
		  		}else if(pastTurn == RIGHT){
		  			TurnCW();
		  		}
		  }
  		for(int i = 1; i < 4; i++){
  			if(theMap[nodeMat[currentEdge[0]][currentEdge[1]]+i*turnDirection][currentEdge[0]] != -1){
  				currentEdge[1] = theMap[nodeMat[currentEdge[0]][currentEdge[1]]+i*turnDirection][currentEdge[0]];
  				break;
  			}
  			if(i == 3){
  				motor.stop_all(); LCD.clear(); LCD.print("bug #88Ø"); delay(20000);//TODO get rid of this delay
  			}
  		}

  	}else if(distFromIntCase == 2 || (leftDiff > farFromIntCount && rightDiff > farFromIntCount && !discrepancyInLocation)){
			switch(currentEdge[1]){
  			case 1: TurnCCW(); break;
  			case 3: TurnCW(); break;
  			default:
		  		if(pastTurn == LEFT){
		  			TurnCCW();
		  		}else if(pastTurn == RIGHT){
		  			TurnCW();
		  		}
		  }  		
			int tempInt180 = currentEdge[1];
			currentEdge[1] = currentEdge[0];
			currentEdge[0] = tempInt180;
  	} else{ //grey zone or already lost
  		motor.stop_all(); LCD.clear(); LCD.print("In grey zone"); delay(1000);
  		if(pastTurn == LEFT){ // Jonah switched these directions???
  			TurnCCW();
  		}else if(pastTurn == RIGHT){
  			TurnCW();
  		}
  		currentEdge[0] = GARBAGE;
  		currentEdge[1] = GARBAGE;
  		leftInitial = GARBAGE;
      rightInitial = GARBAGE;
  		discrepancyInLocation = true;
  	}
  }
  pastTurn = pastTurn*-1; //Switch pastDirection so that next turn is in proper direction
	leftEncoderAtLastInt = leftCount;
	rightEncoderAtLastInt = rightCount;
  LCD.clear();LCD.print("From: "); LCD.print(currentEdge[0]); LCD.print(" To: ");LCD.print(currentEdge[1]);
  //motor.stop_all(); delay(1000);
}

void TurnCCW(){
	TurnAround(LEFT_MOTOR, RIGHT_MOTOR, leftCount, rightCount);	// Set currentEdge for special cases - These currentEdges are the already flipped ones as if the turn had been completed on a single edge successfull
	
	// special cases for nodes we know are dead ends
	/*if(currentEdge[0] == 5 && currentEdge[1] == 6){
		currentEdge[0] = 6;
		currentEdge[1] = 11; 
	} else if(currentEdge[0] == 2 && currentEdge[1] == 7){
		currentEdge[0] = 7;
		currentEdge[1] = 12; 
	}	

	if(!passengerSpotted){
		TurnDecision();
	}*/
}

void TurnCW(){
	TurnAround(RIGHT_MOTOR, LEFT_MOTOR, rightCount, leftCount);
	
	// Special cases for nodes we know are dead ends. CurrentEdges are flipped ones as if the turn had been completed on a single edge successfully
	/*if(currentEdge[0] == 9 && currentEdge[1] == 8){
		currentEdge[0] = 8;
		currentEdge[1] = 14; 
	} else if(currentEdge[0] == 2 && currentEdge[1] == 7){
		currentEdge[0] = 7;
		currentEdge[1] = 13; 
	}	

	if(!passengerSpotted){
		TurnDecision();
	}*/
}

void CollisionCheck(){
	if((digitalRead(OR) || digitalRead(FRONT_LEFT_BUMPER_PIN)) && !collisionDetected){
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
	int reverseCount = 0;
	int initialEncoderCount = encoderCount;
	while(encoderCount - initialEncoderCount < 100){
		reverseCount++;
		motor.speed(fastMotor, -MAX_MOTOR_SPEED*1/2);
		motor.speed(slowMotor, -MAX_MOTOR_SPEED*1/3);
		if(digitalRead(q1) || digitalRead(q2) /*|| reverseCount > 100000*/){ //TODO: Uncomment this
			break;
		}
	}
}

void ReverseRight(){
	ReverseSlow(LEFT_MOTOR, RIGHT_MOTOR, leftCount);
}
void ReverseLeft(){
	ReverseSlow(RIGHT_MOTOR, LEFT_MOTOR, rightCount);
}