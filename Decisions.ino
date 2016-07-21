void TurnDecision(){
  if(hasPassenger){
    currentDir = (nodeMat[currentEdge[1]][currentEdge[0]] + 2) % 4;
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
      while(true){} //get rid of this before competiton. good for now.
    }
  }
  else if(!discrepancyInLocation){
	  highestProfit = 0;
	  for(int dir = 0; dir < 4; dir++){
	    nextTempNode = theMap[dir][currentEdge[1]];
	    if(nextTempNode != -1){
	      tempInt = profitMatrix[currentEdge[1]][nextTempNode]; //change of temp int!
	      if(tempInt > highestProfit){
	        highestProfit = tempInt;
	        desiredDirection = dir;
	      }
	    }
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
  desiredTurn = desiredTurns[turnCount];
  turnCount++;
}
