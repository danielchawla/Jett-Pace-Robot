clear;
clc;
theMap = [
                  % 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19
                  [-1,-1,-1,-1,-1,-1, 1, 2, 3,-1, 0, 6, 7, 7, 8, 4,10,11,14,15];
                  [-1,-1,-1,-1,-1, 6,-1,13, 9,-1,11,12,-1,14,15,-1,17,18,19,-1];
                  [10, 6, 7, 8,15,-1,11,-1,14,-1,16,17,13,12,18,19,-1,-1,-1,-1];
                  [-1,-1,-1,-1,-1,-1, 5,12,-1, 8,-1,10,11,-1,13,14,-1,16,17,18]];

for i = 1:4
    for j = 1:20
      if theMap(i,j) ~= -1
        nodeMat(j,theMap(i,j)+1) = i;
      end
    end
end

stuckLikelyhood = [ 8, 8, 8, 8, 8, 8, 9, 4, 9, 8, 5, 1, 4, 4, 1, 5, 4, 2, 2, 4];
distToDropoff =   [ 4, 4, 5, 4, 4, 4, 3, 4, 3, 4, 3, 2, 3, 3, 2, 3, 2, 1, 1, 2];

for i = 1:4
    for j = 1:20
        if theMap(i,j) > 0
            initialProfitMatrix(i,j) = 100 - 10*distToDropoff(theMap(i,j)+1) - 8*stuckLikelyhood(theMap(i,j)+1)/2;
        else
            initialProfitMatrix(i,j) = -100;
        end
    end
end
currentEdge0 = 0;
currentEdge1 = 10;

profitMatrix = initialProfitMatrix;

for node = 1:50
    highestProfit = 0;
    
    profitMatrix(nodeMat(currentEdge0+1,currentEdge1+1),currentEdge0+1) = 0;
    profitMatrix(nodeMat(currentEdge1+1,currentEdge0+1),currentEdge1+1) = 0;
    
    for dir = 1:4
        for nextnode = 1:20
            if profitMatrix(dir,nextnode) + initialProfitMatrix(dir,nextnode)/30 <= initialProfitMatrix(dir,nextnode)
              profitMatrix(dir,nextnode) = profitMatrix(dir,nextnode) + initialProfitMatrix(dir,nextnode)/30;
            end
        end
        tempInt = profitMatrix(dir,currentEdge1+1);
        if tempInt > highestProfit
            highestProfit = tempInt;
            desiredDirection = dir;
        end
        
    end
    
    %output stuff
    path(node) = currentEdge1
    for dir = 1:4
        profits(dir) = profitMatrix(dir,currentEdge1+1);
    end
    desiredDirection
    profits
    %inp = input('');
    %reset
    currentEdge0 = currentEdge1;
    currentEdge1 = theMap(desiredDirection,currentEdge1+1);
    
   
end

path

