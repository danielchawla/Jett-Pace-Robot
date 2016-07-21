void TurnAround(){

	motor.stop_all();
	//Back up a bit
	motor.speed(LEFT_MOTOR, -1*vel/3);
	motor.speed(RIGHT_MOTOR, -1*vel/3);
	countLeft180 = leftCount;
	countRight180 = rightCount;
	while((leftCount - countLeft180 < 30) && (rightCount - countRight180 < 30)){}
	motor.speed(LEFT_MOTOR, 0);
	motor.speed(RIGHT_MOTOR, 0);

	// Reverse just left motor for ?? revolutions
	motor.speed(LEFT_MOTOR, -1*MAX_MOTOR_SPEED/2*3);
	motor.speed(RIGHT_MOTOR, -1*MAX_MOTOR_SPEED/4);
	countLeft180 = leftCount;
	while(leftCount - countLeft180 < 80){}
	motor.speed(LEFT_MOTOR, 0);
	motor.speed(RIGHT_MOTOR, 0);	

	// Drive right motor until finds tape
	motor.speed(RIGHT_MOTOR, MAX_MOTOR_SPEED/2*3);
	while(true){
		if(digitalRead(q1)){
			statusCount180++;
		}
		if(statusCount180 > 10){
			break;
		}
	}
	motor.speed(RIGHT_MOTOR, 0);
	statusCount180 = 0;
	// Reset current direction
  int temp = currentEdge[1];
  currentEdge[1] = currentEdge[0];
  currentEdge[0] = temp;
}