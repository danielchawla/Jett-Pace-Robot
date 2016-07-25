clear;
close all
clc;
hasPassenger = 0;
passengerSpotted = 0;
passengerEdges = [13,14;12,11];
theMap = [
    % 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19
    [-1,-1,-1,-1,-1,-1, 1, 2, 3,-1, 0, 6, 7, 7, 8, 4,10,11,14,15];
    [-1,-1,-1,-1,-1, 6,-1,13, 9,-1,11,12,-1,14,15,-1,17,18,19,-1];
    [10, 6, 7, 8,15,-1,11,-1,14,-1,16,17,13,12,18,19,-1,-1,-1,-1];
    [-1,-1,-1,-1,-1,-1, 5,12,-1, 8,-1,10,11,-1,13,14,-1,16,17,18]];

dirToDropoff        = [3, 3, 3, 3, 3, 2, 3, 2, 3, 4, 3, 3, 4, 2, 3, 3, 2, 2, 4, 4]; 
secondDirToDropoff  = [3, 3, 3, 3, 3, 2, 1, 4, 1, 4, 2, 4, 3, 3, 2, 4, 1, 1, 1, 1];

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
    % Decision Making Process at Each Node:
    %   1. Reset profitability of current edge if passenger not spotted
    %   2. Update all other edge profitabilities
    %   3. If has passenger go towards dropoff
    %   4. If no passenger, go where most profitable
    %   5. If spotted passenger and has passenger, increase profitability
    %       in opposite direction to HIGH
    
    if hasPassenger && (currentEdge0 == 17 || currentEdge0 == 18)
        hasPassenger = 0;
        if passengerSpotted
            temp = currentEdge0;
            currentEdge0 = currentEdge1;
            currentEdge1 = temp
            disp('turning around')
            pastDirection = mod(pastDirection+2, 4);
            passengerSpotted = 0;
        end
    end
    
    % 1
    if(~passengerSpotted)          
        profitMatrix(nodeMat(currentEdge0+1,currentEdge1+1),currentEdge0+1) = 0;
        profitMatrix(nodeMat(currentEdge1+1,currentEdge0+1),currentEdge1+1) = 0;
    end
    % 2
    for dir = 1:4
        for nextnode = 1:20
            if profitMatrix(dir,nextnode) < initialProfitMatrix(dir,nextnode)
                profitMatrix(dir,nextnode) = profitMatrix(dir,nextnode) + floor(initialProfitMatrix(dir,nextnode)/30)+1;
                if(profitMatrix(dir,nextnode) > initialProfitMatrix(dir,nextnode))
                    profitMatrix(dir,nextnode) = initialProfitMatrix(dir,nextnode);
                end
            end
        end
    end
    % 3
    if(hasPassenger)
        desiredDirection = dirToDropoff(currentEdge1+1);
        if abs(mod(nodeMat(currentEdge1+1,currentEdge0+1)+1,4)+1 - pastDirection)  == 2
            desiredDirection = secondDirToDropoff(currentEdge1+1)
        end
    % 4
    else
        highestProfit = 0;
        
        for dir = 1:4
            tempInt = profitMatrix(dir,currentEdge1+1);
            if tempInt > highestProfit && theMap(dir, currentEdge1+1) ~= currentEdge0 + 1
                highestProfit = tempInt;
                desiredDirection = dir;
            end
            
        end
        %output stuff
    end
    
    for dir = 1:4
        profits(dir) = profitMatrix(dir,currentEdge1+1);
    end
    pastDirection = desiredDirection;
    state = [hasPassenger, passengerSpotted]
    path(node) = currentEdge1
    
    inp = input('');
    
    if(inp == -1)
        temp = currentEdge0;
        currentEdge0 = currentEdge1;
        currentEdge1 = temp;
        pastDirection = mod(pastDirection+2, 4)
    else
        currentEdge0 = currentEdge1;
        currentEdge1 = theMap(desiredDirection,currentEdge1+1);
    end
    
    % check if there is a passenger on this edge and update hasPassenger
    % and passengerSpotted
    if ismember([currentEdge1,currentEdge0], passengerEdges, 'rows') || ismember([currentEdge0,currentEdge1], passengerEdges, 'rows')
        if ~hasPassenger
            if ismember([currentEdge1,currentEdge0], passengerEdges, 'rows')
               passengerEdges(find(passengerEdges(:,1) == currentEdge1),:) = [-1,-1];
            else
                passengerEdges(find(passengerEdges(:,2) == currentEdge1),:) = [-1,-1];
            end
        end
        
        disp('seen passenger')
        if ~hasPassenger
            hasPassenger = 1;
        else
            passengerSpotted = 1
        end
    end
    % 5
    if(passengerSpotted)
        disp('I should get here')
        profitMatrix(nodeMat(currentEdge1+1,currentEdge0+1), currentEdge1+1) = 300;
    end
    
    
end
figure
histogram(path)
path


