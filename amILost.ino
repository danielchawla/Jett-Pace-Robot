void amILost(void){
	topIR0 = analogRead(ir0);
  topIR1 = analogRead(ir1);
  topIR2 = analogRead(ir2);

  directionOfDropZone = -1;
  if(topIR0 > topIRSensitivity){
  	directionOfDropZone = 90;
  }
  else if(topIR1 > topIRSensitivity){
  	directionOfDropZone = 180;
  }
  else if(topIR2 > topIRSensitivity){
  	directionOfDropZone = 270;
  }

  //just after the intersection at currentNode (currentEdge[0]),
  dirAfterInt = nodeMat[currentEdge[0]][currentEdge[1]];
  robotDirection = (bearingToDropoff[currentEdge[0]] - directionOfDropZone + 360) % 360;
  tempInt = (dirAfterInt*90 -robotDirection + 360) % 360; //change of temp int! should be close to 0 if we know our location correctly.

  tempInt = 0; // FOR TESTING!!!!!!!!!

  if(!(tempInt < accuracyInIR || tempInt > 360 - accuracyInIR)){
    discrepancyInLocation = true;
  }
  else{
  	discrepancyInLocation = false;
  }
}