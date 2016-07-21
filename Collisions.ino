#define stage1 15
#define stage2 70
#define stage3 80
void TurnAround(){
	//Stage1: Reverse just left
	int stage = 1;
	LCD.clear();LCD.print("Stage 1");
	countLeft180 = leftCount;
	motor.speed(LEFT_MOTOR, -1*MAX_MOTOR_SPEED*2/3);
	while(true){
		if(stage == 1 && (leftCount - countLeft180) > stage1){
			stage++;
			motor.stop_all();
			delay(1000);
			//Entering Stage2: Reverse both
			LCD.clear();LCD.print("Stage 2 "); LCD.print(leftCount - countLeft180);
		  countLeft180 = leftCount;
			motor.speed(LEFT_MOTOR, -1*MAX_MOTOR_SPEED*2/3);
			motor.speed(RIGHT_MOTOR, -1*MAX_MOTOR_SPEED*5/16);
		}
		if(stage == 2 && leftCount - countLeft180 > stage2){
			stage++;
			motor.stop_all();
			delay(1000);
			// Entering Stage 3: Pivot
			LCD.clear();LCD.print("Stage 3 "); LCD.print(leftCount - countLeft180);
			countLeft180 = leftCount;
			motor.speed(LEFT_MOTOR, -1*MAX_MOTOR_SPEED/3);
			motor.speed(RIGHT_MOTOR, MAX_MOTOR_SPEED*2/3);
		}
		if(stage == 3 && leftCount - countLeft180 > stage3){
			stage++;
			motor.stop_all();
			delay(1000);
			// Entering stage 4: Reverse left slow, drive right
			LCD.clear();LCD.print("Stage 4 "); LCD.print(leftCount - countLeft180);
			countLeft180 = leftCount;
			motor.speed(LEFT_MOTOR, -1*MAX_MOTOR_SPEED*3/8);
			motor.speed(RIGHT_MOTOR, MAX_MOTOR_SPEED*2/3);
		}

		//Check if tape is found
		if((digitalRead(q1) || digitalRead(q2)) && stage > 2){// Need to change the stage > condition - not adaptable enough
			statusCount180++;
		} else if(statusCount180 && stage > 2){
			statusCount180--;
		}
		if(statusCount180 > 15){
			statusCount180 = 0;
			stage = 1; // Reset stage
			break;
		}

		/*CollisionCheck();
		if(collisionDetected && stage != 1){
			//For testing: enter menu if a collision while turning
			MainMenu();
			collisionDetected = false;
		}*/



		/*
		//Stage2: Reverse both
	  countLeft180 = leftCount;
		motor.speed(LEFT_MOTOR, -1*MAX_MOTOR_SPEED/2*3);
		motor.speed(RIGHT_MOTOR, -1*MAX_MOTOR_SPEED*3/8);
		while(leftCount - countLeft180 < stage2){}

		//Stage3: Pivot
		motor.speed(LEFT_MOTOR, -1*MAX_MOTOR_SPEED/2*3);
		motor.speed(RIGHT_MOTOR, MAX_MOTOR_SPEED*3/2);
		while(leftCount - countLeft180 < stage3){}

		//Stage4: Reverse left slower, drive right
		motor.speed(LEFT_MOTOR, -1*MAX_MOTOR_SPEED/2*3);
		motor.speed(RIGHT_MOTOR, MAX_MOTOR_SPEED*3/2);
		while(leftCount - countLeft180 < stage3){}
			*/
	}
	/*int reversing = 1;
	int wheelStuckCount = 0;
	motor.stop_all();

	//Back up a bit
	motor.speed(LEFT_MOTOR, -1*vel/3);
	motor.speed(RIGHT_MOTOR, -1*vel/3);
	countLeft180 = leftCount;
	countRight180 = rightCount;
	//while((leftCount - countLeft180 < 5) || (rightCount - countRight180 < 5)){}
	motor.speed(LEFT_MOTOR, 0);
	motor.speed(RIGHT_MOTOR, 0);
	delay(1000);

	countLeft180 = leftCount;
	while(true){
		// Reverse just left motor for ?? revolutions
		if(reversing){
			motor.speed(LEFT_MOTOR, -1*MAX_MOTOR_SPEED/2*3);
			while(leftCount - countLeft180 < 135){
				if(leftCount - countLeft180 > 60 || leftCount - countLeft180 < 30)
					{motor.speed(RIGHT_MOTOR, 0);
				}else{
					motor.speed(RIGHT_MOTOR, -1*MAX_MOTOR_SPEED/8*3);
				}
				//LCD.clear();LCD.print("Stuck");
			}
		}else{ // Turn forward
			// Drive right motor until finds tape
			motor.speed(RIGHT_MOTOR, MAX_MOTOR_SPEED/2*3);
			countRight180 = rightCount;
			while(true){
				if(rightCount - countRight180 > 60 || rightCount - countRight180 < 30)
				
				if(statusCount180 > 10){
					break;
				}
			}
			motor.speed(RIGHT_MOTOR, 0);
		}

		//Check if found tape
		if(digitalRead(q1)){
			statusCount180++;
		}
		if(statusCount180 > 10){
			bread
		}
	}
	statusCount180 = 0;
	// Reset current direction
  int temp = currentEdge[1];
  currentEdge[1] = currentEdge[0];
  currentEdge[0] = temp;
  */

}

void CollisionCheck(){
	if(digitalRead(OR) && !collisionDetected){
    collisionCount++;
    if(collisionCount > 20){
      collisionDetected = true;
      collisionCount = 0;
      switchVals[FRONT_BUMPER] = digitalRead(FRONT_BUMPER_PIN);
      switchVals[FRONT_RIGHT_BUMPER] = digitalRead(FRONT_RIGHT_BUMPER_PIN);
      switchVals[RIGHT_BUMPER] = digitalRead(RIGHT_BUMPER_PIN);
      switchVals[REAR_BUMPER] = digitalRead(REAR_BUMPER_PIN);
      switchVals[LEFT_BUMPER] = digitalRead(LEFT_BUMPER_PIN);
      switchVals[FRONT_LEFT_BUMPER] = digitalRead(FRONT_LEFT_BUMPER_PIN);
    }
  } else if(collisionCount){
    collisionCount--;
  }
}