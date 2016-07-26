void determineLocation(void){
	if(lastKnownIntersection == GARBAGE){
		lastKnownIntersection = currentEdge[0]; //crucial that this is true. It would be if determineLocation is called after collision
		if(nodeMat[currentEdge[0]][currentEdge[1]] == (nodeMat[currentEdge[1]][currentEdge[0]] +2)%4){ //if we're on a straight edge
			lastKnownDir = nodeMat[currentEdge[0]][currentEdge[1]];
			
		}
		else{
			//Leave it to Jonah
		}





	}
}