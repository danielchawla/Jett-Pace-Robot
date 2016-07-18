void TurnDecision(){
  topIR0 = analogRead(ir0);
  topIR1 = analogRead(ir1);
  topIR2 = analogRead(ir2);

  directionOfDropZone = -1;
  if(topIR0 > topIRSensitivity){
  	directionOfDropZone = 90;
  }
  else if(topIR1 > topIRSensitivity){
  	directionOfDropZone = 210;
  }
  else if(topIR2 > topIRSensitivity){
  	directionOfDropZone = 330;
  }

  //just after the intersection at currentNode (currentEdge[0]),
  currentDir = nodeMat[currentEdge[0]][currentEdge[1]];
  robotDirection = (bearingToDropoff[currentEdge[0]] - directionOfDropZone + 360) % 360;
  tempInt = (currentDir*90 -robotDirection + 360) % 360; //change of temp int! should be close to 0 if we know our location correctly.
  if(!(tempInt < accuracyInIR || tempInt > 360 - accuracyInIR)){
    discrepancyInLocation = true;
  }

  if(!discrepancyInLocation){
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
    currentDir = (nodeMat[currentEdge[1]][currentEdge[0]] + 2) % 4;
    desiredTurn = desiredDirection - currentDir;
    switch (desiredTurn){
      case 3: desiredTurn = -1; break;
      case -3: desiredTurn = 1; break;
      case -2: desiredTurn = 2; break; 
    }
  }
  else{
  	discrepancyInLocation = false; //maybe if we can confidently reslove the issue.
    if (directionOfDropZone > 0){
    	//we can do something about this. it increases our confidence.
    } 
    
  }


    // For testing, turn left, right, straight, left ...
    desiredTurn = desiredTurns[turnCount];
    turnCount++;
    if(turnCount == 15){
      motor.stop_all();
      while(true){}
    }
   // turnCount = (turnCount) % 15;
}
