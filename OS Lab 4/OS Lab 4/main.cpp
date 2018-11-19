//
//  main.cpp
//  OS Lab 4
//
//  Created by Danny Tan on 4/8/18.
//  Copyright © 2018 Danny Tan. All rights reserved.
//

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>

using namespace std;


class Page {
public:
    Page(int size) {
        valid = false;
        data = -1;
        time = -1;
        processNum = -1;
        this->size = size;
    }
    
    bool contain(int address, int processNum) {
        //check if the page contains for the address, first check if valid, then if the process number are the same and lastly compare the address
        return valid && (this->processNum == processNum) && (address>=this->data&&address<this->data+size);
    }
    
    bool isValid() const {
        return valid;
    }
    
    
    int getTime() const {
        return time;
    }
    
    void setTime(int time) {
        this->time = time;
    }
    
    int getTimeAdded() const {
        return timeAdded;
    }
    
    void setValid(bool valid) {
        this->valid = valid;
    }
    void setTimeAdded(int time) {
        timeAdded = time;
    }
    
    void setProcessNum(int num) {
        processNum = num;
    }
    
    int getProcessNum() const {
        return processNum;
    }
    
    void setData(int data) {
        this->data = data;
    }
    
private:
    bool valid;
    int data, time, timeAdded, processNum, size;
};

class Process {
public:
    
    Process() {
        numOfHit = 0;
        numOfFaults = 0;
        evictions = 0;
        residencyTime = 0;
    }
    int getNumHit() const {
        return numOfHit;
    }
    int getNumFault() const {
        return numOfFaults;
    }
    
    void increaseNumHit() {
        numOfHit++;
    }
    void increaseNumFault() {
        numOfFaults++;
    }
    
    double getresidencyTime() const {
        return residencyTime;
    }
    void setresidencyTime(double time) {
        residencyTime = time;
    }
    void increaseEvictions() {
        evictions++;
    }
    int getEvictions() const {
        return evictions;
    }
    
private:
    int numOfHit, numOfFaults, evictions;
    double residencyTime;
};



int findPage(vector<Page> pages, string replacementAlgo, int fifoCount, int randNum) {
    //find the page that is invalid
    
    for(int i = pages.size()-1;i>=0;i--){
        if(!pages[i].isValid()){
            return i;
        }
    }
    
    //if all page are valid
    if(replacementAlgo == "lru"){
        
        int res = -1;
        int min = INT_MAX;
        
        //find the index of the page with the least time reference
        for(int i=0;i<pages.size();i++){
            if(pages[i].getTime()<=min){
                min=pages[i].getTime();
                res=i;
            }
        }
        return res;
    }
    //random
    if(replacementAlgo == "random"){
        return randNum%(pages.size());
    }
    
    //fifo
    if (replacementAlgo == "fifo") {
        //reset fifo if 0
        if(fifoCount==0){
            fifoCount=pages.size()-1;
            return fifoCount;
        }
        return --fifoCount;
    }
    return -1;

}

//print out result total miss, total residency time
void printOut(vector<Process> vectorOfProcess) {
    int totalMisses=0;
    double totalResidencyTime=0,totalEvictions=0;
    for(int i=0;i<4;i++){
        if(vectorOfProcess[i].getEvictions()==0){
            cout << "Process " << i + 1 << " had " << vectorOfProcess[i].getNumFault() << " faults. With no evictions, the average residence is undefined." << endl;
            totalMisses+=vectorOfProcess[i].getNumFault();
        }else{
            cout << "Process " << i + 1 << " had " << vectorOfProcess[i].getNumFault() << " faults and " << vectorOfProcess[i].getresidencyTime()/vectorOfProcess[i].getEvictions() << " average residency" << endl;
            totalMisses += vectorOfProcess[i].getNumFault();
            totalResidencyTime += vectorOfProcess[i].getresidencyTime();
            totalEvictions += vectorOfProcess[i].getEvictions();
        }
    }
    if(totalEvictions==0){
        cout << "The total number of faults is " << totalMisses << ". With no evictions, the overall average residence is undefined." << endl;
    }else{
        cout << "The total number of faults is "<< totalMisses << " and the overall average residency is " << totalResidencyTime/totalEvictions << endl;
    }

}

// JobMix = 1
void jobMix1(vector<Page> pages, int pageSize, int processSize, int numRefPerProcess, int fifoCount, string replacementAlgo) {
    
    ifstream randomFile;
    
    randomFile.open("random.txt");
    if (!randomFile){
        cerr << "Could not open random file" << endl;
    }
    
    int currentRunningNum=0, currentRunningAddress = (111+processSize)%processSize;
    Process process = Process();

    for(int i = 0; i<numRefPerProcess; i++){
        bool found = false;
        int randNum;
        //check if a page if found and hit
        for(int j = 0;j<pages.size();j++){
            if(pages[j].contain(currentRunningAddress,0)){
                pages[j].setTime(i);
                process.increaseNumHit();
                found=true;
            }
        }
        //if none of the page is hit
        if(!found){
            
            //gets random number from file
            bool valid = true;
            for(int i = pages.size()-1;i>=0;i--){
                valid = valid && pages[i].isValid();
            }
            if (valid && replacementAlgo == "random") {
                randomFile >> randNum;
            }
            
            //find the free using the respective algo
            int freePageNum=findPage(pages, replacementAlgo, fifoCount, randNum);
            fifoCount=freePageNum;
            //increase the number of miss
            process.increaseNumFault();
            //if the page was valid, increase residency time and eviction
            if(pages[freePageNum].isValid()){
                process.setresidencyTime(process.getresidencyTime() + i -pages[freePageNum].getTimeAdded());
                process.increaseEvictions();
            }
            //set the data for the new free page
            pages[freePageNum].setValid(true);
            pages[freePageNum].setTime(i);
            pages[freePageNum].setTimeAdded(i);
            pages[freePageNum].setData(currentRunningAddress-(currentRunningAddress+pageSize)%pageSize);
            pages[freePageNum].setProcessNum(currentRunningNum);
        }
        currentRunningAddress += 1;
        currentRunningAddress = (currentRunningAddress+processSize)%processSize;
        
        randomFile >> randNum;
        
    }
    if(process.getEvictions()==0){
        cout << "Process 1 had " << process.getNumFault() << " faults. With no evictions, the average residence is undefined." << endl << "The total number of faults is " << process.getNumFault() << ". With no evictions, the overall average residence is undefined." << endl;
    }else{
        cout << "Process 1 had " << process.getNumFault() << " faults and " << process.getresidencyTime()/process.getEvictions() << " average residency" << endl << "The total number of faults is " << process.getNumFault() << " and the overall average residency is " << process.getresidencyTime()/process.getEvictions() << endl;
    }
    
    randomFile.close();
}



// JobMix = 2
void jobMix2(vector<Page> pages, int pageSize, int processSize, int numRefPerProcess,int fifoCount, string replacementAlgo) {
    
    ifstream randomFile;
    
    randomFile.open("random.txt");
    if (!randomFile){
        cerr << "Could not open random file" << endl;
    }
    
    
    int currentRunningNum=0;
    vector<int> vectorOfCurrentAddress;
    vector<Process> vectorOfProcess;
    
    //create processes
    for(int i=0;i<4;i++){
        Process process = Process();
        vectorOfProcess.push_back(process);
        vectorOfCurrentAddress.push_back((111*(i+1)+ processSize)%processSize);
    }
    
    
    for(int i = 0;i<(numRefPerProcess-(numRefPerProcess+3)%3)*4; i++){
        bool found=false;
        int randNum = 0;
        
         //check if a page if found and hit
        for(int j=0;j<pages.size();j++){
            if(pages[j].contain(vectorOfCurrentAddress[currentRunningNum],currentRunningNum)){
                pages[j].setTime(i);
                vectorOfProcess[currentRunningNum].increaseNumHit();
                found=true;
            }
        }
        //if none of the page is hit
        if(!found){
           
            bool valid = true;
            for(int i = pages.size()-1;i>=0;i--){
                valid = valid && pages[i].isValid();
            }
            if (valid && replacementAlgo == "random") {
                randomFile >> randNum;
            }
             //find the free using the respective algo
            int freePageNum=findPage(pages, replacementAlgo, fifoCount,randNum);
            fifoCount=freePageNum;
            vectorOfProcess[currentRunningNum].increaseNumFault();
            //if the page was valid, increase residency time and eviction
            if(pages[freePageNum].isValid()){
                vectorOfProcess[pages[freePageNum].getProcessNum()].setresidencyTime(vectorOfProcess[pages[freePageNum].getProcessNum()].getresidencyTime() + i - pages[freePageNum].getTimeAdded());
                vectorOfProcess[pages[freePageNum].getProcessNum()].increaseEvictions();
            }
            //set the data for the new free page
            pages[freePageNum].setValid(true);
            pages[freePageNum].setTime(i);
            pages[freePageNum].setTimeAdded(i);
            pages[freePageNum].setData(vectorOfCurrentAddress[currentRunningNum] - (vectorOfCurrentAddress[currentRunningNum]+ pageSize) % pageSize);
            pages[freePageNum].setProcessNum(currentRunningNum);
        }
        vectorOfCurrentAddress[currentRunningNum]++;
        vectorOfCurrentAddress[currentRunningNum] = (vectorOfCurrentAddress[currentRunningNum] + processSize) % processSize;
        
        //quantum q=3
        if(i%3==2){
            currentRunningNum += 1;
            currentRunningNum = (currentRunningNum+4)%4;
        }
        
        randomFile >> randNum;
    }
    for(int i = (numRefPerProcess-(numRefPerProcess+3)%3)*4;i<numRefPerProcess*4;i++){
        bool found=false;
        int randNum = -1;
        
        //check if a page if found and hit
        for(int j=0;j<pages.size();j++){
            if(pages[j].contain(vectorOfCurrentAddress[currentRunningNum],currentRunningNum)){
                pages[j].setTime(i);
                vectorOfProcess[currentRunningNum].increaseNumHit();
                found=true;
            }
        }
         //if none of the page is hit
        if(!found){
            
            bool valid = true;
            for(int i = pages.size()-1;i>=0;i--){
                valid = valid && pages[i].isValid();
            }
            if (valid && replacementAlgo == "random") {
                randomFile >> randNum;
            }
            
             //find the free using the respective algo
            int freePageNum=findPage(pages, replacementAlgo, fifoCount,randNum);
            fifoCount=freePageNum;
            
            
               //if the page was valid, increase residency time and eviction
            vectorOfProcess[currentRunningNum].increaseNumFault();
            if(pages[freePageNum].isValid()){
                vectorOfProcess[pages[freePageNum].getProcessNum()].setresidencyTime(vectorOfProcess[pages[freePageNum].getProcessNum()].getresidencyTime() + i - pages[freePageNum].getTimeAdded());
                vectorOfProcess[pages[freePageNum].getProcessNum()].increaseEvictions();
            }
            
            //set the data for the new free page
            pages[freePageNum].setValid(true);
            pages[freePageNum].setTime(i);
            pages[freePageNum].setTimeAdded(i);
            pages[freePageNum].setData(vectorOfCurrentAddress[currentRunningNum] - (vectorOfCurrentAddress[currentRunningNum]+ pageSize) % pageSize);
            pages[freePageNum].setProcessNum(currentRunningNum);
        }
        
        vectorOfCurrentAddress[currentRunningNum]++;
        vectorOfCurrentAddress[currentRunningNum] = (vectorOfCurrentAddress[currentRunningNum] + processSize) % processSize;
        
    
        //quantum q=3
        if(i%(numRefPerProcess%3)==(numRefPerProcess%3)-1){
            currentRunningNum += 1;
            currentRunningNum = (currentRunningNum+4)%4;
        }
        
        randomFile >> randNum;
        
    }
    printOut(vectorOfProcess);
    randomFile.close();
}

// JobMix = 3

void jobMix3(vector<Page> pages, int pageSize, int processSize, int numRefPerProcess,int fifoCount, string replacementAlgo) {
    
    ifstream randomFile;
    
    randomFile.open("random.txt");
    if (!randomFile){
        cerr << "Could not open random file" << endl;
    }
    
    int currentRunningNum=0;
    vector<int> vectorOfCurrentAddress;
    vector<Process> vectorOfProcess;
    for(int i=0;i<4;i++){
        Process process = Process();
        vectorOfProcess.push_back(process);
        vectorOfCurrentAddress.push_back((111*(i+1)+ processSize)%processSize);
    }
    
    
    for(int i = 0;i<(numRefPerProcess-(numRefPerProcess+3)%3)*4; i++){
        bool found=false;
        
        //check if a page if found and hit
        for(int j=0;j<pages.size();j++){
            if(pages[j].contain(vectorOfCurrentAddress[currentRunningNum],currentRunningNum)){
                pages[j].setTime(i);
                vectorOfProcess[currentRunningNum].increaseNumHit();
                found=true;
            }
        }
        
        //if none of the page is hit
        if(!found){
            bool valid = true;
            int randNum = 0;
            for(int i = pages.size()-1;i>=0;i--){
                valid = valid && pages[i].isValid();
            }
            if (valid && replacementAlgo == "random") {
                randomFile >> randNum;
            }
            
            
            //find the free using the respective algo
            int freePageNum=findPage(pages, replacementAlgo, fifoCount,randNum);
            fifoCount=freePageNum;
            vectorOfProcess[currentRunningNum].increaseNumFault();
            //if the page was valid, increase residency time and eviction
            if(pages[freePageNum].isValid()){
                vectorOfProcess[pages[freePageNum].getProcessNum()].setresidencyTime(vectorOfProcess[pages[freePageNum].getProcessNum()].getresidencyTime() + i - pages[freePageNum].getTimeAdded());
                vectorOfProcess[pages[freePageNum].getProcessNum()].increaseEvictions();
            }
              //set the data for the new free page
            pages[freePageNum].setValid(true);
            pages[freePageNum].setTime(i);
            pages[freePageNum].setTimeAdded(i);
            pages[freePageNum].setData(vectorOfCurrentAddress[currentRunningNum] - (vectorOfCurrentAddress[currentRunningNum]+ pageSize) % pageSize);
            pages[freePageNum].setProcessNum(currentRunningNum);
        }
        
        int randNum;
        randomFile >> randNum;
        
        vectorOfCurrentAddress[currentRunningNum] = (randNum+processSize) % processSize;
        
        //quantum q=3
        if(i%3==2){
            currentRunningNum++;
            currentRunningNum = (currentRunningNum+4)%4;
        }
    }
    
    
    for(int i = (numRefPerProcess-(numRefPerProcess+3)%3)*4;i<numRefPerProcess*4;i++){
        bool found=false;
        
        //check if a page if found and hit
        for(int j=0;j<pages.size();j++){
            if(pages[j].contain(vectorOfCurrentAddress[currentRunningNum],currentRunningNum)){
                pages[j].setTime(i);
                vectorOfProcess[currentRunningNum].increaseNumHit();
                found=true;
            }
        }
         //if none of the page is hit
        if(!found){
            bool valid = true;
            int randNum = 0;
            for(int i = pages.size()-1;i>=0;i--){
                valid = valid && pages[i].isValid();
            }
            if (valid && replacementAlgo == "random") {
                randomFile >> randNum;
            }
            //find the free using the respective algo
            int freePageNum=findPage(pages, replacementAlgo, fifoCount,randNum);
            fifoCount=freePageNum;
            vectorOfProcess[currentRunningNum].increaseNumFault();
             //if the page was valid, increase residency time and eviction
            if(pages[freePageNum].isValid()){
                vectorOfProcess[pages[freePageNum].getProcessNum()].setresidencyTime(vectorOfProcess[pages[freePageNum].getProcessNum()].getresidencyTime() + i - pages[freePageNum].getTimeAdded());
                vectorOfProcess[pages[freePageNum].getProcessNum()].increaseEvictions();
            }
              //set the data for the new free page
            pages[freePageNum].setValid(true);
            pages[freePageNum].setTime(i);
            pages[freePageNum].setTimeAdded(i);
            pages[freePageNum].setData(vectorOfCurrentAddress[currentRunningNum] - (vectorOfCurrentAddress[currentRunningNum]+ pageSize) % pageSize);
            pages[freePageNum].setProcessNum(currentRunningNum);
        }
        int randNum;
        randomFile >> randNum;
         vectorOfCurrentAddress[currentRunningNum] = (randNum+processSize) % processSize;
        
        //quantum q=3
        if(i%(numRefPerProcess%3)==(numRefPerProcess%3)-1){
            currentRunningNum++;
            currentRunningNum = (currentRunningNum+4)%4;
        }
    }

    printOut(vectorOfProcess);
    randomFile.close();
}


// JobMix = 4
void jobMix4(vector<Page> pages, int pageSize, int processSize, int numRefPerProcess,int fifoCount, string replacementAlgo) {
    
    ifstream randomFile;
    
    randomFile.open("random.txt");
    if (!randomFile){
        cerr << "Could not open random file" << endl;
    }
    
    int currentRunningNum=0;
    vector<int> vectorOfCurrentAddress;
    vector<Process> vectorOfProcess;
    for(int i=0;i<4;i++){
        Process process = Process();
        vectorOfProcess.push_back(process);
        vectorOfCurrentAddress.push_back((111*(i+1)+ processSize)%processSize);
    }
    for(int i = 0;i<(numRefPerProcess-(numRefPerProcess+3)%3)*4; i++){
        bool found=false;
        for(int j=0;j<pages.size();j++){
            if(pages[j].contain(vectorOfCurrentAddress[currentRunningNum],currentRunningNum)){
                pages[j].setTime(i);
                vectorOfProcess[currentRunningNum].increaseNumHit();
                found=true;
            }
        }
        if(!found){
            bool valid = true;
            int randNum = 0;
            for(int i = pages.size()-1;i>=0;i--){
                valid = valid && pages[i].isValid();
            }
            if (valid && replacementAlgo == "random") {
                randomFile >> randNum;
            }
            int freePageNum=findPage(pages, replacementAlgo, fifoCount,randNum);
            fifoCount=freePageNum;
            vectorOfProcess[currentRunningNum].increaseNumFault();
            if(pages[freePageNum].isValid()){
                vectorOfProcess[pages[freePageNum].getProcessNum()].setresidencyTime(vectorOfProcess[pages[freePageNum].getProcessNum()].getresidencyTime() + i - pages[freePageNum].getTimeAdded());
                vectorOfProcess[pages[freePageNum].getProcessNum()].increaseEvictions();
            }
            pages[freePageNum].setValid(true);
            pages[freePageNum].setTime(i);
            pages[freePageNum].setTimeAdded(i);
            pages[freePageNum].setData(vectorOfCurrentAddress[currentRunningNum] - (vectorOfCurrentAddress[currentRunningNum]+ pageSize) % pageSize);
            pages[freePageNum].setProcessNum(currentRunningNum);
        }

        int randNum;
        randomFile >> randNum;
        double y= randNum/(INT_MAX +1.0);
        switch (currentRunningNum) {
            case 0:
                if (y<0.75) {
                    vectorOfCurrentAddress[currentRunningNum] = (vectorOfCurrentAddress[currentRunningNum] + 1 + processSize) % processSize;
                }
                else {
                    vectorOfCurrentAddress[currentRunningNum] = (vectorOfCurrentAddress[currentRunningNum] - 5 + processSize) % processSize;
                }
                break;
            case 1:
                if (y<0.75) {
                    vectorOfCurrentAddress[currentRunningNum] = (vectorOfCurrentAddress[currentRunningNum] + 1 + processSize) % processSize;
                }
                else {
                    vectorOfCurrentAddress[currentRunningNum] = (vectorOfCurrentAddress[currentRunningNum] + 4 + processSize) % processSize;
                }
                break;
            case 2:
                if (y<0.75) {
                    vectorOfCurrentAddress[currentRunningNum] = (vectorOfCurrentAddress[currentRunningNum] + 1 + processSize) % processSize;
                }
                else if (y < 0.875) {
                    vectorOfCurrentAddress[currentRunningNum] = (vectorOfCurrentAddress[currentRunningNum] - 5 + processSize) % processSize;
                }
                else {
                    vectorOfCurrentAddress[currentRunningNum] = (vectorOfCurrentAddress[currentRunningNum] + 4 + processSize) % processSize;
                }
                break;
            case 3:
                if (y<0.5) {
                    vectorOfCurrentAddress[currentRunningNum] = (vectorOfCurrentAddress[currentRunningNum] + 1 + processSize) % processSize;
                }
                else if (y < 0.625) {
                    vectorOfCurrentAddress[currentRunningNum] = (vectorOfCurrentAddress[currentRunningNum] - 5 + processSize) % processSize;
                }
                else if (y < 0.75){
                    vectorOfCurrentAddress[currentRunningNum] = (vectorOfCurrentAddress[currentRunningNum] + 4 + processSize) % processSize;
                }
                else {
                    int randNum;
                    randomFile >> randNum;
                    vectorOfCurrentAddress[currentRunningNum] = (randNum+processSize) % processSize;
                }
                break;
            default:
                break;
        }
   
        if(i%3==2){
            currentRunningNum++;
            currentRunningNum = (currentRunningNum+4)%4;
        }
    }
    for(int i = (numRefPerProcess-(numRefPerProcess+3)%3)*4;i<numRefPerProcess*4;i++){
        bool found=false;
        for(int j=0;j<pages.size();j++){
            if(pages[j].contain(vectorOfCurrentAddress[currentRunningNum],currentRunningNum)){
                pages[j].setTime(i);
                vectorOfProcess[currentRunningNum].increaseNumHit();
                found=true;
            }
        }
        if(!found){
            bool valid = true;
            int randNum = 0;
            for(int i = pages.size()-1;i>=0;i--){
                valid = valid && pages[i].isValid();
            }
            if (valid && replacementAlgo == "random") {
                randomFile >> randNum;
            }
            int freePageNum=findPage(pages, replacementAlgo, fifoCount,randNum);
            fifoCount=freePageNum;
            vectorOfProcess[currentRunningNum].increaseNumFault();
            if(pages[freePageNum].isValid()){
                vectorOfProcess[pages[freePageNum].getProcessNum()].setresidencyTime(vectorOfProcess[pages[freePageNum].getProcessNum()].getresidencyTime() + i - pages[freePageNum].getTimeAdded());
                vectorOfProcess[pages[freePageNum].getProcessNum()].increaseEvictions();
            }
            pages[freePageNum].setValid(true);
            pages[freePageNum].setTime(i);
            pages[freePageNum].setTimeAdded(i);
            pages[freePageNum].setData(vectorOfCurrentAddress[currentRunningNum] - (vectorOfCurrentAddress[currentRunningNum]+ pageSize) % pageSize);
            pages[freePageNum].setProcessNum(currentRunningNum);
        }
        int randNum;
        randomFile >> randNum;
        double y= randNum/(INT_MAX +1.0);
        switch (currentRunningNum) {
            case 0:
                if (y<0.75) {
                    vectorOfCurrentAddress[currentRunningNum] = (vectorOfCurrentAddress[currentRunningNum] + 1 + processSize) % processSize;
                }
                else {
                    vectorOfCurrentAddress[currentRunningNum] = (vectorOfCurrentAddress[currentRunningNum] - 5 + processSize) % processSize;
                }
                break;
            case 1:
                if (y<0.75) {
                    vectorOfCurrentAddress[currentRunningNum] = (vectorOfCurrentAddress[currentRunningNum] + 1 + processSize) % processSize;
                }
                else {
                    vectorOfCurrentAddress[currentRunningNum] = (vectorOfCurrentAddress[currentRunningNum] + 4 + processSize) % processSize;
                }
                break;
            case 2:
                if (y<0.75) {
                    vectorOfCurrentAddress[currentRunningNum] = (vectorOfCurrentAddress[currentRunningNum] + 1 + processSize) % processSize;
                }
                else if (y < 0.875) {
                    vectorOfCurrentAddress[currentRunningNum] = (vectorOfCurrentAddress[currentRunningNum] - 5 + processSize) % processSize;
                }
                else {
                    vectorOfCurrentAddress[currentRunningNum] = (vectorOfCurrentAddress[currentRunningNum] + 4 + processSize) % processSize;
                }
                break;
            case 3:
                if (y<0.5) {
                    vectorOfCurrentAddress[currentRunningNum] = (vectorOfCurrentAddress[currentRunningNum] + 1 + processSize) % processSize;
                }
                else if (y < 0.625) {
                    vectorOfCurrentAddress[currentRunningNum] = (vectorOfCurrentAddress[currentRunningNum] - 5 + processSize) % processSize;
                }
                else if (y < 0.75){
                    vectorOfCurrentAddress[currentRunningNum] = (vectorOfCurrentAddress[currentRunningNum] + 4 + processSize) % processSize;
                }
                else {
                    int randNum;
                    randomFile >> randNum;
                    vectorOfCurrentAddress[currentRunningNum] = (randNum+processSize) % processSize;
                }
                break;
            default:
                break;
        }
        
        if(i%(numRefPerProcess%3)==(numRefPerProcess%3)-1){
            currentRunningNum++;
            currentRunningNum = (currentRunningNum+4)%4;
        }
    }
    
    printOut(vectorOfProcess);
    randomFile.close();
}


int main(int argc, const char * argv[]) {
    int machineSize, pageSize, processSize, jobMixNum, numRefPerProcess;
    string replacementAlgorithm;

    //extract data from command line
    machineSize = atoi(argv[1]);
    pageSize = atoi(argv[2]);
    processSize = atoi(argv[3]);
    jobMixNum = atoi(argv[4]);
    numRefPerProcess = atoi(argv[5]);
    replacementAlgorithm = argv[6];
    
    cout << "The machine size is " << machineSize << endl;
    cout << "The page size is " << pageSize << endl;
    cout << "The process size is " << processSize << endl;
    cout << "The job mix number is " << jobMixNum << endl;
    cout << "The number of references per process is " << numRefPerProcess << endl;
    cout << "The replacement algorithm is " << replacementAlgorithm << endl;
    
    //find the number of pages
    int numberOfPages = machineSize/pageSize;
    vector<Page> pages;
    for (int i = 0; i < numberOfPages; i++) {
        pages.push_back(Page(pageSize));
    }
    int fifoCount = -1;
    
    switch(jobMixNum) {
        case 1:
            jobMix1(pages, pageSize, processSize, numRefPerProcess, fifoCount, replacementAlgorithm);
            break;
        case 2:
            jobMix2(pages, pageSize, processSize, numRefPerProcess, fifoCount, replacementAlgorithm);
            break;
        case 3:
            jobMix3(pages, pageSize, processSize, numRefPerProcess, fifoCount, replacementAlgorithm);
            break;
        case 4:
            jobMix4(pages, pageSize, processSize, numRefPerProcess, fifoCount, replacementAlgorithm);
            break;
        default:
            break;
    }
}
