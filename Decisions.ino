void TurnDecision1(){
  currentDir = (nodeMat[currentEdge[1]][currentEdge[0]] + 2) % 4;//direction with which we will enter the next intersection.

  // ADDED FOLLOWING CODE BEFOR MAIN IF(HASPASSENGER)
    // Should still update profit matrices even when carrying passenger back to base - need to increase profits though if a passenger is spotted - Can I do this in here???
  if(!passengerSpotted ){ // 1 in MATLAB
    profitMatrix[nodeMat[currentEdge[0]][currentEdge[1]]][currentEdge[0]] = 0;
    profitMatrix[nodeMat[currentEdge[1]][currentEdge[0]]][currentEdge[1]] = 0; // Should this one still go to 0 if a passenger is spotted
  }else{ // 5 in MATLAB
    profitMatrix[nodeMat[currentEdge[0]][currentEdge[1]]][currentEdge[0]] = 0;
    profitMatrix[nodeMat[currentEdge[1]][currentEdge[0]]][currentEdge[1]] = 100;
  }
  for (int i = 0; i <4; i++){ // 2 in MATLAB
    for (int j = 0; j<20; j++){ // Increment the profitabilities of all other edges by 1/40?? of their initial value
      if(profitMatrix[i][j] < initialProfitMatrix[i][j]){
        profitMatrix[i][j]+= initialProfitMatrix[i][j]/40 + 1; //the +1 is to avoid adding 0.  Ryan I though /40 messed everything up???
        if(profitMatrix[i][j] > initialProfitMatrix[i][j]){
          profitMatrix[i][j] = initialProfitMatrix[i][j];
        }
      }
    }
  }
}
void TurnDecision2(){
  // Make actual decision
  if(hasPassenger && !discrepancyInLocation){ // 3 in MATLAB
    desiredDirection = dirToDropoff[currentEdge[1]]; // Ideal direction to dropoff

    if((desiredDirection - currentDir+4)%4 == 2){ // If ideal direction requres 180 turn use secondary direction - could maybe scrap this if 180s are reliable
      desiredDirection = secondDirToDropoff[currentEdge[1]]; 
    }

    desiredTurn = desiredDirection - currentDir; // Map desiredDirection and currentDirection to a turn direction (LEFT, RIGHT, STRAIGHT)
    switch (desiredTurn){
      case 3: desiredTurn = LEFT; break;
      case -3: desiredTurn = RIGHT; break;
      case -2: desiredTurn = BACK; break; 
    }
  }
  else if(!discrepancyInLocation){  // 4 in MATLAB
    // If no passenger and we're not lost, seek most profitable route
  	//same as MATLAB robotNav.m (hopefully)
	  highestProfit = 0;

    // Make the profit along the current edge 0 - Will never want to turn around
		/*profitMatrix[nodeMat[currentEdge[0]][currentEdge[1]]][currentEdge[0]] = 0;
		profitMatrix[nodeMat[currentEdge[1]][currentEdge[0]]][currentEdge[1]] = 0;*/

		for (int i = 0; i <4; i++){
	    /*for (int j = 0; j<20; j++){ // Increment the profitabilities of all other edges by 1/40?? of their initial value
	      if(profitMatrix[i][j] < initialProfitMatrix[i][j]){
	        profitMatrix[i][j]+= initialProfitMatrix[i][j]/40 + 1; //the +1 is to avoid adding 0.  Ryan I though /40 messed everything up???
          if(profitMatrix[i][j] > initialProfitMatrix[i][j]){
            profitMatrix[i][j] = initialProfitMatrix[i][j];
          }
	      }
	    }*/
      profits[i] = profitMatrix[i][currentEdge[1]]; //change of temp int!
      if(profits[i] > highestProfit && theMap[i][currentEdge[1]] != currentEdge[0]){ // Added && to not go backwards when passengerSpottedd
        highestProfit = profits[i];
        desiredDirection = i; // Find the direction with the highest profit
      }
    }


    desiredTurn = desiredDirection - currentDir; // Map two directions to a turn direction
    switch (desiredTurn){
      case 3: desiredTurn = LEFT; break;
      case -3: desiredTurn = RIGHT; break;
      case -2: desiredTurn = BACK; break; 
    }
  }
 	else{ // If we are lost, we have to do something about it
 		desiredTurn = STRAIGHT; //this is okay to stay. We are doing something about it right after intersections.
	}

      
  // For testing, turn left, right, straight, left ...
  desiredTurn = desiredTurns[turnCount];
  turnCount++;
}
