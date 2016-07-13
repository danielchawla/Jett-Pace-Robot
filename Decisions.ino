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


  currentDir = (nodeMat[currentEdge[0]][currentEdge[1]] + 2) % 4;
  currentNode = currentEdge[0];




  // For testing, turn left, right, straight, left ...
  desiredTurn = desiredTurns[turnCount];
  turnCount++;
}