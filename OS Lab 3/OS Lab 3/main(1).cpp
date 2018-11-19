//
//  main.cpp
//  OS Lab 3
//
//  Created by Danny Tan on 3/18/18.
//  Copyright © 2018 Danny Tan. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>


using namespace std;


//an activity consit of instruction, delay time, resource, and units claimed

class Activity {
public:
    Activity(string instruction, int delay, int resource, int unitsClaimed) {
        this->instruction = instruction;
        this->delay = delay;
        this->resource = resource;
        this-> unitsClaimed = unitsClaimed;
    }
    
    
    string getInstruction() const {
        return instruction;
    }
    
    int getDelayTime() const {
        return delay;
    }
    
    int getResource() const {
        return resource;
    }
    
    int getUnitsClaimed() const {
        return unitsClaimed;
    }
    
private:
    string instruction;
    int delay;
    int resource;
    int unitsClaimed;
    
    
};



class FIFO {
    public:
    
    //constructor
    FIFO(int numberOfTask, int numberOfResource, vector<int> availableUnits, vector<vector<Activity>> tasks) {
        this->numberOfTask = numberOfTask;
        this->numberOfResource = numberOfResource;
        this->availableUnits = availableUnits;
        this->tasks = tasks;
    }
    
    
    void increaseDelayTime(int currentTask) {
        ++delayTime[currentTask];
    }
    
    
    void eraseTask(int currentTask) {
        tasks[currentTask].erase(tasks[currentTask].begin());
    }
    
    void increaseTimeTaken(int currentTask) {
        ++timeTaken[currentTask];
    }
    
    void increaseWaitTime(int currentTask) {
        ++waitTime[currentTask];
    }
    
    int getRemainingTask() {
        return numberOfTask - finishedTask;
    }
    
    void abort(int index) {
        timeTaken[index] = -1; // abort since time is -1
        
        // and release all of the respective resources
        for (int j = 0; j < numberOfResource; j++) {
            availableUnits[j] += runningTasks[index][j];
            setResourceOfCurrentTask(index, j, 0);
        }
        
        //clear that task and increase the number of finish task
        tasks[index].clear();
    }
    
    void setResourceOfCurrentTask (int currentTask, int resource, int value) {
        runningTasks[currentTask][resource] = value;
    }
    
    
    void setAvailableUnits(int resource, int value){
        availableUnits[resource] = value;
    }
    
    
    void setReleasedUnits( int resource, int value) {
        releasedUnits[resource] = value;
    }
    
    void increaseFinishTask() {
        finishedTask += 1;
    }
    
    void runTask() {
        
        bool deadlock;
        int deadlockCount = 0;
        finishedTask = 0;
        
        
        //initialize
        
        for(int i = 0; i < numberOfTask; i++) {
            timeTaken.push_back(0);
            delayTime.push_back(0);
            waitTime.push_back(0);
            
            vector<int> temp;

            for (int j = 0; j < numberOfResource; j++) {
                temp.push_back(0);
            }
            
            runningTasks.push_back(temp);
        }
        
        for (int i = 0; i < numberOfResource; i++) {
            releasedUnits.push_back(0);
        }
        
        //check if the task are finished running
        
        while (finishedTask < numberOfTask) {
            deadlock = false;
            
            //check for deadlocks
            
            //deadlock occurs when deadlock count = the number of remaining task, nothing can run
            if (deadlockCount == getRemainingTask()) {
                
                deadlock = true;
                
                for (int i = 0; i < numberOfTask; i++) {
                    
                    //abort the first task if deadlock
                    if ( !tasks[i].empty() ) {
                        abort(i);
                        
                        increaseFinishTask();
                        break;
                    }
                }
            }
            
            
            
            //reset deadlock count
            deadlockCount = 0;
            
            //ready queue
            
            for (int i = 0; i < numberOfTask; i++) {
                //check if it is ready in queue and if all of the task are finished running
                if (find(ready.begin(), ready.end(), i) == ready.end()) {
                    ready.push_back(i);
                }
            }
            
        
            
            //remaining task
            
            for (int i = 0; i < getRemainingTask(); i++) {
                
                
                //pick up the first activity from ready queue
                int currentTask = ready[0];
    
                // check if the currentTask state has already been processed
                if ( !tasks[currentTask].empty() ) {
                    
                    string instruction = tasks[currentTask][0].getInstruction();
                    int delay = tasks[currentTask][0].getDelayTime();
                    int resource = tasks[currentTask][0].getResource();
                    int claim = tasks[currentTask][0].getUnitsClaimed();
                
                    
                    
                    //check if activity is delayed
                    if (delay != delayTime[currentTask]) {
                        increaseDelayTime(currentTask);
                        increaseTimeTaken(currentTask);
                        
                    } else {
                        delayTime[currentTask] = 0;
                     
                       
                        if (instruction == "initiate") {
                            // initiate a new task
                            
                            
                            
                            increaseTimeTaken(currentTask);
                            
                            setResourceOfCurrentTask(currentTask, resource - 1, 0);
                            
                            eraseTask(currentTask);
                            
                            break;
                        }
                        else if (instruction == "request") {
                            
                            // request resources
                            
                            
                            // if claim asked is greater than the available unit
                            if (claim > availableUnits[resource - 1]) {
                                
                                //check if the currentTask state is deadlock
                                if (!deadlock) {
                                    increaseTimeTaken(currentTask);
                                    increaseWaitTime(currentTask);
                                }
                                //add back into ready list to run again next time
                                ready.push_back(currentTask);
                                deadlockCount++;
                                
                                //else request available units
                            } else {
                                setResourceOfCurrentTask(currentTask, resource - 1, runningTasks[currentTask][resource - 1] + claim);
                                
                                setAvailableUnits(resource - 1, availableUnits[resource-1]-claim);
                                increaseTimeTaken(currentTask);
                                
                                eraseTask(currentTask);
                                
                            }
                            

                        }
                        
                        else if (instruction == "release") {
                            // release resources
                            
                            
                            //put into a release vector to put back into avaiable units later
                            setReleasedUnits(resource -1, releasedUnits[resource - 1] + claim);
                            setResourceOfCurrentTask(currentTask, resource - 1, runningTasks[currentTask][resource - 1] - claim);
                            increaseTimeTaken(currentTask);
                            eraseTask(currentTask);
                            

                        }
                        
                        else {
                            // if task is terminate
                            
                            //increase the number of finish task if terminate
                            increaseFinishTask();
                            eraseTask(currentTask);

                        }
                        
                    }
                }
                
                //if processed, go back to the previous running state
                else {
                    i--;
                }
                
                //go to the next ready task
                
                ready.erase(ready.begin());
            }
            
            
            //put back all of the release units into available units and reset released units
            for (int i = 0; i < numberOfResource; i++) {
                setAvailableUnits(i, availableUnits[i] + releasedUnits[i]);
                setReleasedUnits(i, 0);
            }
        }
    }
    
    
    
    //print out the result
    void result() {
        
        cout << "FIFO" << endl;
        
        double totalWaitTime = 0;
        double totalRunTime = 0;
        
        for (int i = 0; i < numberOfTask; i++) {
            
            cout << "Task" << i + 1 << "\t";
            
            if (timeTaken[i] == -1) {
                cout << "aborted  " << endl;
                
            }
            else {
                
                cout <<  timeTaken[i] << "\t" << waitTime[i] << "\t" <<  double(waitTime[i])/ double(timeTaken[i]) * 100 << "%" << endl;
                
                totalRunTime += timeTaken[i];
                totalWaitTime += waitTime[i];
                
            }
        }
        
        cout << "Total\t" << totalRunTime << "\t" << totalWaitTime << "\t" << totalWaitTime/totalRunTime * 100 << "%" << endl;

    }
    
private:
    int numberOfTask;
    int numberOfResource;
    vector<int> timeTaken;
    vector<int> waitTime;
    vector<int> availableUnits;
    vector<vector<Activity>> tasks;
    vector<int> delayTime;
    vector<int>releasedUnits;
    vector<int> ready;
    vector<vector<int>> runningTasks;
    int finishedTask;
};




class Banker {
    public:
    
    //constructor
    Banker(int numberOfTask, int numberOfResource, vector<int> availableUnits, vector<vector<Activity>> tasks) {
        this->numberOfTask = numberOfTask;
        this->numberOfResource = numberOfResource;
        this->availableUnits = availableUnits;
        this->tasks = tasks;
    }
    
    
    
    void increaseDelayTime(int currentTask) {
        delayTime[currentTask] += 1;
    }
    
    void resetDelayTime(int currentTask) {
        delayTime[currentTask] = 0;
    }
    
    
    void eraseTask(int currentTask) {
        tasks[currentTask].erase(tasks[currentTask].begin());
    }
    
    void increaseTimeTaken(int currentTask) {
        timeTaken[currentTask] += 1;
    }
    
    void increaseWaitTime(int currentTask) {
        waitTime[currentTask] += 1;
    }

    
    void abort(int index, int i, int resource) {
        timeTaken[index] = -1; // abort since time is -1
        
        // and release all of the respective resources
        for (int j = 0; j < numberOfResource; j++) {
            setReleasedUnits(j, releasedUnits[j] + runningTasks[index][j]);
            setAvailableUnits(j, availableUnits[j] + getPretendClaim(index,resource));
            setResourceOfCurrentTask(i,j,0);
        }
        
        //clear that task and increase the number of finish task
        tasks[index].clear();
    }
    
    void setResourceOfCurrentTask (int currentTask, int resource, int value) {
        runningTasks[currentTask][resource] = value;
    }
    
    
    void setAvailableUnits(int resource, int value){
        availableUnits[resource] = value;
    }
    
    
    void setReleasedUnits( int resource, int value) {
        releasedUnits[resource] = value;
    }
    
    void increaseFinishTask() {
        finishedTask += 1;
    }
    
    
    
    void setPretendClaimUnits(int task, int resource, int value) {
        pretendClaim[task][resource] = value;
    }
    
    
    int getPretendClaim(int task, int resource) const {
        return pretendClaim[task][resource];
    }
    
    
    int getAvailableUnits(int resource) const {
        return availableUnits[resource];
    }
    
    void runTask() {
        
        int safe = 0;
        finishedTask = 0;
        int cycle = 0;
        
       
        
        
        //initialize
        
        for(int i = 0; i < numberOfTask; i++) {
            timeTaken.push_back(0);
            delayTime.push_back(0);
            waitTime.push_back(0);
            
            vector<int> temp;
            
            for (int j = 0; j < numberOfResource; j++) {
                temp.push_back(0);
            }
            
            runningTasks.push_back(temp);
            pretendClaim.push_back(temp);
            pretendGive.push_back(temp);
        }
        
        
        for (int i = 0; i < numberOfResource; i++) {
            releasedUnits.push_back(0);
        }
        
        //check if the task are finished running
        
        while (finishedTask < numberOfTask) {
          
        
            //ready queue
            
            for (int i = 0; i < numberOfTask; i++) {
                //check if it is ready in queue and if all of the task are finished running
                if (find(ready.begin(), ready.end(), i) == ready.end()) {
                    ready.push_back(i);
                }
            }
            
            
            //remaining task
            int numberOfRemainingTask = numberOfTask - finishedTask;
            
            
            for (int i = 0; i < numberOfRemainingTask; i++) {
                
                
                //pick up the first activity from ready queue
                int currentTask = ready[0];
                
                // check if the currentTask state has already been processed
                if ( !tasks[currentTask].empty() ) {
                    string instruction = tasks[currentTask][0].getInstruction();
                    int delay = tasks[currentTask][0].getDelayTime();
                    int resource = tasks[currentTask][0].getResource();
                    int claim = tasks[currentTask][0].getUnitsClaimed();
                    
                    
                    //check if activity is delayed
                    if (delay != delayTime[currentTask]) {
                        increaseTimeTaken(currentTask);
                        increaseDelayTime(currentTask);
                        
                    } else {
                        
        
                        if (instruction == "initiate") {
                            
                            // initiate a new task
                            
                            
                            
                            //if claim asked is less than or equal to the available units, safe
                            if (claim <= getAvailableUnits(resource-1)) {
                                
                                
                                //pretend to claim, check if safe later
                                
                                setPretendClaimUnits(currentTask, resource -1, -claim);
                                
                                pretendGive[currentTask][resource - 1] = claim;
                                
                                resetDelayTime(currentTask);
                                
                                setResourceOfCurrentTask(currentTask,resource -1, 0);
                                
                                increaseTimeTaken(currentTask);
                                eraseTask(currentTask);
                            }
                            
                            else {
                                
                                
                                
                                //abort
                                cout << "Banker aborts task " << currentTask + 1 << " before run begins: " << endl;
                                cout << "claim for resourse " << resource << " (" << claim << ") exceeds number of units present (" << getAvailableUnits(resource-1) << ")" << endl;
                                
                                
                                timeTaken[currentTask] = -1;
                                tasks[currentTask].clear();
                                increaseFinishTask();
                                
                                
                            }

                        }
                        else if (instruction == "request") {
                            // request resources
                            
                            
                            // if claim asked is greater than the available unit and pretend claim to claim earlier, access is denied and put back into ready queue
                            if (claim > getAvailableUnits(resource-1) && getPretendClaim(currentTask,resource -1) < 0) {
                                
                                increaseTimeTaken(currentTask);
                                increaseWaitTime(currentTask);
                                
                                
                                //add back into ready list to run again next time
                                ready.push_back(currentTask);
                                
                            }
                            else {
                                
                                //if more avaiable units than the claim
                                
                                setResourceOfCurrentTask(currentTask, resource -1,   runningTasks[currentTask][resource - 1] + claim);
                                
                                increaseTimeTaken(currentTask);
                                
                                eraseTask(currentTask);
                                
                                //check if pretend claim is bigger than actual claim
                                
                                if (getPretendClaim(currentTask,resource -1) >= 0) {
                                    
                                    setPretendClaimUnits(currentTask, resource-1, getPretendClaim(currentTask,resource-1) - claim);
                                    //pretend/request claim is bigger than actual claim, not safe
                                    if (getPretendClaim(currentTask, resource -1) < 0) {
                                        
                                        //abort task and release all of the respective resource of the currentTask task
                                        abort(currentTask, i, resource -1);
                                        
                                        
                                        
                                        cout << "During cycle " << cycle << "-" << cycle + 1 << " of Banker's algorithms" << endl;
                                        cout << "Task " << i << "'s request exceeds its claim; aborted; " << pretendGive[currentTask][resource-1] + getPretendClaim(currentTask, resource-1) << " units available next cycle" << endl;
                                        
                                        //abort so the task is finished
                                        
                                        increaseFinishTask();
                                        
                                    }
                                    
                                    
                                }
                                
                                //claim is granted, transfer pretend claim to available
                                
                                else {
                                    
                                    
                                    setAvailableUnits(resource -1, getAvailableUnits(resource - 1) - claim);
                                    setPretendClaimUnits(currentTask, resource -1, getPretendClaim(currentTask,resource - 1) + claim);
                                    
                                    
                                    
                                    
                                    
                                }
                                
                                //loop through see if all of the pretend claims are safe
                                
                                for (int k = 0; k < numberOfResource; k++) {
                                    //only safe if the available units is bigger than the requested claims
                                    int claim = getPretendClaim(currentTask,k);
                                    if (claim < 0) {
                                        claim *= -1;
                                    }
                                    if (getAvailableUnits(k) >= claim) {
                                        safe++;
                                    }
                                }
                                
                                //banker request from available units if all claims are safe
                                if (safe == numberOfResource) {
                                    for (int k = 0; k < numberOfResource; k++) {
                                        int claim = getPretendClaim(currentTask,k);
                                        if (claim < 0) {
                                            claim *= -1;
                                        }
                                        
                                        setPretendClaimUnits(currentTask,k,claim);
                                        
                                        setAvailableUnits(k, availableUnits[k] - pretendClaim[currentTask][k]);
                                    }
                                    
                                }
                                
                                //reset safe count
                                safe = 0;
                                resetDelayTime(currentTask);
                                
                            }

                            
                        }
                        
                        else if (instruction == "release") {
                            // release resources
                            
                            resetDelayTime(currentTask);
                            
                            
                            //release all of the request claims
                            if (getPretendClaim(currentTask, resource -1) >= 0) {
                                setPretendClaimUnits(currentTask,resource - 1, getPretendClaim(currentTask,resource - 1) * -1);
                                
                                
                                //loop through to check if all of the request claims are true
                                for (int k = 0; k < numberOfResource; k++) {
                                    int claim = getPretendClaim(currentTask,k);
                                    if (claim < 0) {
                                        claim *= -1;
                                    }
                                    if (getPretendClaim(currentTask,k) <= 0 || claim == pretendGive[currentTask][k] ){
                                        safe++;
                                    }
                                }
                                
                                //see if all of the allocated resource are safe, release back into available units if safe
                                if (safe == numberOfResource) {
                                    
                                    setReleasedUnits(resource -1, releasedUnits[resource - 1] + claim);
                                    
                                    for (int k = 0; k < numberOfResource; k++) {
                                        if (getPretendClaim(currentTask,k) > 0) {
                                            setPretendClaimUnits(currentTask,k, getPretendClaim(currentTask,k) * -1);
                                        }
                                        
                                        setAvailableUnits(k, getAvailableUnits(k) - getPretendClaim(currentTask,k));
                                    }
                                }
                                
                                
                                //reset safe count
                                safe = 0;
                                
                            }
                            
                            
                            setResourceOfCurrentTask(currentTask, resource -1 , runningTasks[currentTask][resource-1] - claim);
                            setPretendClaimUnits(currentTask, resource -1, getPretendClaim(currentTask, resource -1) - claim);
                            
                            increaseTimeTaken(currentTask);
                            eraseTask(currentTask);
                            

                        }
                        
                        else {
                            //if process terminates
                            
                            resetDelayTime(currentTask);
                            increaseFinishTask();
                            eraseTask(currentTask);

                        }
                        
                    }

                }
                
                //if processed, go back to the previous running state
                else {
                    i--;
                }
                
                //go to the next ready task
                
                ready.erase(ready.begin());
            }
            
            
            //put back all of the release units into available units and reset released units
            for (int i = 0; i < numberOfResource; i++) {
                setAvailableUnits(i, availableUnits[i] + releasedUnits[i]);
                setReleasedUnits(i, 0);
            }
            
            cycle ++;
        }
    }
    
    
    
    //print out the result
    void result() {
        
        cout << "BANKER'S" << endl;
        
        double totalWaitTime = 0;
        double totalRunTime = 0;
        
        for (int i = 0; i < numberOfTask; i++) {
            
            cout << "Task" << i + 1 << "\t";
            
            if (timeTaken[i] == -1) {
                cout << "aborted  " << endl;
                
            }
            else {
                
                cout <<  timeTaken[i] << "\t" << waitTime[i] << "\t" <<  double(waitTime[i])/ double(timeTaken[i]) * 100 << "%" << endl;
                
                totalRunTime += timeTaken[i];
                totalWaitTime += waitTime[i];
                
            }
        }
        
        cout << "Total\t" << totalRunTime << "\t" << totalWaitTime << "\t" << totalWaitTime/totalRunTime * 100 << "%" << endl;
        
    }
    
private:
    int finishedTask;
    int numberOfTask;
    int numberOfResource;
    vector<int> timeTaken;
    vector<int> waitTime;
    vector<int> availableUnits;
    vector<vector<Activity>> tasks;
    vector<int> delayTime, releasedUnits, ready;
    vector<vector<int>> runningTasks;
    vector<vector<int>> pretendClaim;
    vector<vector<int>> pretendGive;
};


int main(int argc, const char * argv[]) {
    ifstream inputFile;
    //check if file is valid
    inputFile.open(argv[1]);
    if (!inputFile){
        cerr << "Could not open input file" << endl;
        return 9;
    }
    
    int numberOfTask, numberOfResource;
    vector<int> availableUnits;
    
    
    //get the number of task and resource
    if (inputFile >> numberOfTask >> numberOfResource) {
        int numberOfUnits;
        //for each resource, get the number of free units
        for (int i = 0; i < numberOfResource; i++) {
            if (inputFile >> numberOfUnits) {
                availableUnits.push_back(numberOfUnits);
            }
        }
    }
    
    vector<vector<Activity>> tasks = vector<vector<Activity>>();
    
    
    for (int i = 0; i < numberOfTask; i++) {
        vector<Activity> task;
        tasks.push_back(task);
    }
    
    
    
    
    string instruction;
    int taskNumber, delay, resource, claim;
    
    //read the file for task and its respective instruction, delay, resource and claim
    
    while (inputFile >> instruction >> taskNumber >> delay >> resource >> claim) {
        //create an acitivity using the input
        Activity activity = Activity(instruction, delay, resource, claim);
        //sort all of the instructions, delays, resources, and claims to its respective task
        tasks[taskNumber-1].push_back(activity);
    }
    
    //run the fifo algorithm
    FIFO fifo = FIFO(numberOfTask,numberOfResource,availableUnits, tasks);
    fifo.runTask();
    
    
    //run the banker algorithm
    Banker banker = Banker(numberOfTask,numberOfResource,availableUnits, tasks);
    banker.runTask();
    
    //result
    fifo.result();
    banker.result();
}
