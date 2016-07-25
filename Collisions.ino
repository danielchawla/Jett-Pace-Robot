// Define number of encoder pulses for left wheel are necessary in each stage
#define stage1 20
#define stage2 120 // maybe 95 and stage1 15 is better
#define stage3 90
void TurnAround(int reverseMotor, int driveMotor, volatile unsigned int &encoderCount){
	/*
		Turning around consists of 4 stages:
			Stage 1 - Reverse just left wheel for a defined # of encoder pulses
			Stage 2 - Reverse left fast and right slightly for a # of left encoder pulses
			Stage 3 - Drive just right motor for a predefined # of left pulses
									I just realized this makes no sense to count left pulses, we should probably change it 
			Stage 4 - Drive just right motor until tape is found

		Currently stage 3 and 4 are very similar - maybe reverse left a bit in either stage
		Currently are looking for the tape in only stage 3 or 4 - Will this help us to avoid turning around only 90° onto different edge????

		The biggest things to change in here are counts for each stage and Left and Right motor speeds in each stage
		Also want to eventually check for other collisions, could probably reset process to a different stage in this case
	*/
	//Stage1: Reverse just left
	int stage = 1;
	LCD.clear();LCD.print("Stage 1");
	count180 = encoderCount;
	motor.speed(reverseMotor, -1*MAX_MOTOR_SPEED*2/3);
	while(true){
		// While loop is set up with if statements for different stages similar to turning code at intersections
		if(stage == 1 && (encoderCount - count180) > stage1){ // This is the condition to leave stage 1 - at this point we write Stage 2 speeds and increment stage
			stage++;
			motor.stop_all();
			//Entering Stage 2: Reverse both
			LCD.clear();LCD.print("Stage 2 "); LCD.print(encoderCount - count180);
		  count180 = encoderCount;
			motor.speed(reverseMotor, -1*MAX_MOTOR_SPEED*2/3);
			motor.speed(driveMotor, -1*MAX_MOTOR_SPEED/6);
		}
		if((stage == 2 && encoderCount - count180 > stage2) || digitalRead(REAR_BUMPER_PIN)){ // should maybe change digital read to be more robust
			stage++;
			motor.stop_all();
			// Entering Stage 3: Pivot
			LCD.clear();LCD.print("Stage 3 "); LCD.print(encoderCount - count180);
			count180 = encoderCount;
			//motor.speed(reverseMotor, -1*MAX_MOTOR_SPEED/3);
			motor.speed(driveMotor, MAX_MOTOR_SPEED*2/3);
		}
		if(stage == 3 && encoderCount - count180 > stage3){
			stage++;
			motor.stop_all();
			// Entering stage 4: Reverse left slow, drive right
			LCD.clear();LCD.print("Stage 4 "); LCD.print(encoderCount - count180);
			count180 = encoderCount;
			// motor.speed(reverseMotor, -1*MAX_MOTOR_SPEED*3/8);
			motor.speed(driveMotor, MAX_MOTOR_SPEED*3/4);
		}

		//Check if tape is found
		if((digitalRead(q1) || digitalRead(q2)) && stage > 2){// Need to change the stage > condition - not adaptable enough, although maybe is very useful information on how far we've turned
			statusCount180++;
		} else if(statusCount180 && stage > 2){
			statusCount180--;
		}
		if(statusCount180 > 10){
			//Finished turning around - change currentEdge
			tempInt = currentEdge[1];
			currentEdge[1] = currentEdge[0];
			currentEdge[0] = tempInt;


			statusCount180 = 0;
			stage = 1; // Reset stage
			break;
		}
	}
}

void TurnCW(){
	TurnAround(LEFT_MOTOR, RIGHT_MOTOR, leftCount);
}

void TurnCCW(){
	TurnAround(RIGHT_MOTOR, LEFT_MOTOR, rightCount);
}

void CollisionCheck(){ 
/*
 	Collision checking code - must see collision 20 times, and an individual switch must have been pressed for 20 of those to be deemed the cause of the collision
	Here we may not even need to use the or, and just look at switches individually
*/
	if(digitalRead(OR) && !collisionDetected){
    collisionCount++;
    switchVals[FRONT_BUMPER] += digitalRead(FRONT_BUMPER_PIN);
    switchVals[FRONT_RIGHT_BUMPER] += digitalRead(FRONT_RIGHT_BUMPER_PIN);
    switchVals[RIGHT_BUMPER] += digitalRead(RIGHT_BUMPER_PIN);
    switchVals[REAR_BUMPER] += digitalRead(REAR_BUMPER_PIN);
    switchVals[LEFT_BUMPER] += digitalRead(LEFT_BUMPER_PIN);
    switchVals[FRONT_LEFT_BUMPER] += digitalRead(FRONT_LEFT_BUMPER_PIN);
    if(collisionCount > 20){
    	switchVals[FRONT_BUMPER] /= 15;
      switchVals[FRONT_RIGHT_BUMPER] /= 15;
      switchVals[RIGHT_BUMPER] /= 15;
      switchVals[REAR_BUMPER] /= 15;
      switchVals[LEFT_BUMPER] /= 15;
      switchVals[FRONT_LEFT_BUMPER] /= 15;
      collisionDetected = true;
      collisionCount = 0;

    }
  } else if(collisionCount){
    collisionCount--;
  }
}