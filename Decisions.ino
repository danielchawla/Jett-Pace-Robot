void TurnDecision(){
  topIR0 = analogRead(ir0);
  topIR1 = analogRead(ir1);
  topIR2 = analogRead(ir2);
  //determine strongest and second strongest values
  if (topIR0 > topIR1 && topIR0 > topIR2) {
    strongest = ir0;
    if (topIR1 > topIR2) {
      secondStrongest = ir1;
    }
    else {
      secondStrongest = ir2;
    }
  }
  else if (topIR1 > topIR2 && topIR1 > topIR0) {
    strongest = ir1;
    if (topIR0 > topIR2) {
      secondStrongest = ir0;
    }
    else {
      secondStrongest = ir2;
    }
  }
  else {
    strongest = ir2;
    if (topIR0 > topIR1) {
      secondStrongest = ir0;
    }
    else {
      secondStrongest = ir1;
    }
  }

  //calculate direction based on ir values
  directionOfDropZone = 120 * strongest;
  secondStrongestVal = analogRead(secondStrongest);
  strongestVal = analogRead(strongest);
  offset = 120.0 * (float)secondStrongestVal / (float)(secondStrongestVal + strongestVal); // Need to be cast as doubles? - analogRead gives int
  if ( (strongest + 1) % 3 == secondStrongest ) {
    directionOfDropZone = (directionOfDropZone + offset + 360) % 360;
  }
  else {
    directionOfDropZone = (directionOfDropZone - offset + 360) % 360;
  }

  //just after the intersection at currentNode (currentEdge[0]),
  currentDir = nodeMat[currentEdge[0]][currentEdge[1]];
  robotDirection = (bearingToDropoff[currentEdge[0]] - directionOfDropZone + 360) % 360;
  tempInt = (currentDir*90 -robotDirection + 360) % 360; //change of temp int!
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
    //idk we're facked
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
