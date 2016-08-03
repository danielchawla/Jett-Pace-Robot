void UpdateProfitMatrix(){
  // profitMatrix[nodeMat[currentEdge[0]][currentEdge[1]]][currentEdge[0]] = 0;
  // profitMatrix[nodeMat[currentEdge[1]][currentEdge[0]]][currentEdge[1]] = 0;
  if(!discrepancyInLocation){}
    if(!passengerSpotted ){ // 1 in MATLAB
      profitMatrix[nodeMat[currentEdge[0]][currentEdge[1]]][currentEdge[0]] = 0;
      profitMatrix[nodeMat[currentEdge[1]][currentEdge[0]]][currentEdge[1]] = 0;
    }else{ // 5 in MATLAB
      motor.stop_all(); delay(1000);
      profitMatrix[nodeMat[currentEdge[0]][currentEdge[1]]][currentEdge[0]] = 0;
      profitMatrix[nodeMat[currentEdge[1]][currentEdge[0]]][currentEdge[1]] = 500;
    }
    for (int i = 0; i <4; i++){
      for (int j = 0; j<20; j++){
        if(profitMatrix[i][j] < initialProfitMatrix[i][j]){
          profitMatrix[i][j]+= initialProfitMatrix[i][j]/40 + 1; //the +1 is to avoid adding 0
          if(profitMatrix[i][j] > initialProfitMatrix[i][j]){
            profitMatrix[i][j] = initialProfitMatrix[i][j];
          }
        }
      }
    }
  }

void TurnDecision(){
  currentDir = (nodeMat[currentEdge[1]][currentEdge[0]] + 2) % 4;//direction with which we will enter the next intersection.
  // TODO: Reorganize this like in dev???
  // Make decision
  if(hasPassenger &&!discrepancyInLocation){
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
  }
  else if(!discrepancyInLocation){
    //same as MATLAB robotNav.m
    highestProfit = GARBAGE;
    desiredDirection = GARBAGE;
   

    for (int i = 0; i <4; i++){
      profits[i] = profitMatrix[i][currentEdge[1]]; //change of temp int!
      if(profits[i] > highestProfit){
        highestProfit = profits[i];
        desiredDirection = i;
      }
    }

    desiredTurn = desiredDirection - currentDir;
    switch (desiredTurn){
      case 3: desiredTurn = LEFT; break;
      case -3: desiredTurn = RIGHT; break;
      case -2: desiredTurn = BACK; break; 
    }
    
  }else{
    desiredTurn = STRAIGHT;
  }    
  // For testing, turn left, right, straight, left ...
  desiredTurn = desiredTurns[turnCount];
  //turnCount++;
}