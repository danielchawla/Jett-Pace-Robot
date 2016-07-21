void TurnDecision(){
  currentDir = (nodeMat[currentEdge[1]][currentEdge[0]] + 2) % 4;

  // Update profit matrix to be 0 on current edge and increase all other values
  for (int i = 0; i <4; i++){
    for (int j = 0; j<20; j++){
      if(profitMatrix[i][j] < initialProfitMatrix[i][j]){
        profitMatrix[i][j]++;
      }
    }
  }    
  profitMatrix[nodeMat[currentEdge[0]][currentEdge[1]]][currentEdge[0]] = 0;
  profitMatrix[nodeMat[currentEdge[1]][currentEdge[0]]][currentEdge[1]] = 0;

  // Make decision
  if(hasPassenger){
    desiredDirection = dirToDropoff[currentEdge[1]];

    if((desiredDirection - currentDir+4)%4 == 2){
      desiredDirection = secondDirToDropoff[currentEdge[1]];
    }

    desiredTurn = desiredDirection - currentDir;
    switch (desiredTurn){
      case 3: desiredTurn = LEFT; break;
      case -3: desiredTurn = RIGHT; break;
      case -2: desiredTurn = BACK; break; 
    }
    if(desiredTurn == BACK){
      LCD.clear();LCD.print("ERR: 180 Turn??");
      motor.stop_all();
      while(true){} //CHANGE get rid of this before competiton. good for now.
    }
  }
  else if(!discrepancyInLocation){
	  highestProfit = GARBAGE;
	  for(int dir = 0; dir < 4; dir++){
      profits[dir] = profitMatrix[dir][currentEdge[1]]; //change of temp int!
      if(profits[dir] > highestProfit && (dir - currentDir+4)%4 != 2){
        highestProfit = profits[dir];
        desiredDirection = dir;
      }
    }
    if(highestProfit == GARBAGE){ // only turn option was 180°
      MainMenu();
    }
	  
    currentDir = (nodeMat[currentEdge[1]][currentEdge[0]] + 2) % 4; //direction with which we will enter the next intersection.
    desiredTurn = desiredDirection - currentDir;
    switch (desiredTurn){
      case 3: desiredTurn = LEFT; break;
      case -3: desiredTurn = RIGHT; break;
      case -2: desiredTurn = BACK; break; 
    }
  }
 	else{
 		if (directionOfDropZone > 0){ //if we're lost and we know the direction of drop zone
	  	//we can do something about this. it increases our confidence.
	  }
	  else{
	  	//we're facked boys. This'll be interesting... stay tuned!
	  }
	}

      
  // For testing, turn left, right, straight, left ...
  //desiredTurn = desiredTurns[turnCount];
  //turnCount++;
}
