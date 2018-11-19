//
//  main.cpp
//  OS Lab1
//
//  Created by Danny Tan on 2/3/18.
//  Copyright © 2018 Danny Tan. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>

using namespace std;

class Instruction{
public:
    Instruction(const char& classification, int address) {
        this->classification = classification;
        this->address = address;
    }
    
    char getClassification() {
        return classification;
    }
    int getAddress() {
        return address;
    }
    
private:
    char classification;
    int address;
};


struct Module {
    vector<Instruction> vectorOfInstruction;
    map<string, int> mapOfSymbolDefined;
    vector<string> vectorOfSymbolUsed;
    int baseAddress;
};


int main(int argc, char *argv[]) {
    
    //check if file is valid
    ifstream inputFile(argv[1]);
    if (!inputFile){
        cerr << "Could not open file" << endl;
        return 9;
    }
    
    int numberOfModule;
    vector<Module> vectorOfModule;
    map<string, int> symbolsDefined;
    set<string> setOfSymbolUsed;
    map<string, string> symbolWithErrors;
    map<string, int> definitionModError;
    
    //scan for the number of module
    if (inputFile >> numberOfModule) {
        
        for (int i = 0; i < numberOfModule ; i++) {
            Module aModule;
            
            int numberOfDefintion, numberOfUse, numberOfInstruction;
            
            //scan the number of definition
            if (inputFile >> numberOfDefintion) {
                for (int i = 0; i < numberOfDefintion; i++ ) {
                    string variable;
                    int value;
                    
                    if (inputFile >> variable >> value) {
                        //check if it was defined before
                        if (symbolsDefined.find(variable) == symbolsDefined.end()) {
                            symbolsDefined[variable] = value;
                            //adds the definition to the map
                            aModule.mapOfSymbolDefined[variable] = value;
                            //no error
                            symbolWithErrors[variable] = "";
                        }
                        //use the first value if it is defined before and print error
                        else {
                            symbolWithErrors[variable] = "Error: This variable is multiply defined; first value used.";
                        }
                    }
                }
            }
            //scan the number of use
            if (inputFile >> numberOfUse) {
                for (int i = 0; i < numberOfUse; i++ ) {
                    string variable;
                    if (inputFile >> variable) {
                        //add the use to map
                        aModule.vectorOfSymbolUsed.push_back(variable);
                        //add the use to the set
                        setOfSymbolUsed.insert(variable);
                    }
                }
            }
            //scan the number of instruction
            if (inputFile >> numberOfInstruction) {
                for (int i = 0; i < numberOfInstruction; i++ ) {
                    //add the instructions to the module
                    char classification;
                    int address;
                    if (inputFile >> classification >> address) {
                        Instruction aInstruction = Instruction(classification, address);
                        aModule.vectorOfInstruction.push_back(aInstruction);
                    }
                }
            }
            
            //compare whether symbol defined is bigger than the module size
            for (auto symbol = aModule.mapOfSymbolDefined.begin(); symbol != aModule.mapOfSymbolDefined.end(); ++symbol) {
                if (symbol->second > numberOfInstruction) {
                    symbol->second = 0;
                    symbolsDefined[symbol->first] = 0;
                    //sure the error with the symbol and the value;
                    definitionModError[symbol->first] = i;
                }
            }
            
            if (vectorOfModule.size() != 0) {
                Module previousModule = vectorOfModule[vectorOfModule.size() -1];
                //current base address is the previous base address + previous number of instructions
                aModule.baseAddress = previousModule.baseAddress + int(previousModule.vectorOfInstruction.size());
            }
            else {
                aModule.baseAddress = 0; //first module
            }
            vectorOfModule.push_back(aModule);
        }
    }
    
    //first pass
    cout << "Symbol Table" << endl;
    for (int i = 0; i < vectorOfModule.size(); i++) {
        //print out the symbol table
        for (auto symbol = vectorOfModule[i].mapOfSymbolDefined.begin(); symbol != vectorOfModule[i].mapOfSymbolDefined.end(); ++symbol) {
            symbol->second += vectorOfModule[i].baseAddress;
            cout << symbol->first << "=" << symbol->second;
            //update the calculated value to the map
            symbolsDefined[symbol->first] = symbol->second;
            //print out error if it was defined multiple times
            cout << " " << symbolWithErrors[symbol->first] << endl;
        }
    }
    
    //second pass
    cout << "Memory Map" << endl;
    int counter = 0;
    int machineSize = 200;
    for (int i = 0; i < vectorOfModule.size(); i++) {
        for (int j = 0; j < vectorOfModule[i].vectorOfInstruction.size(); j++) {
            cout << counter << ":      ";
            string error = "";
            int addressWithOpCode = vectorOfModule[i].vectorOfInstruction[j].getAddress();
            int opcode = addressWithOpCode/1000;
            int address = addressWithOpCode % 1000;
            switch (vectorOfModule[i].vectorOfInstruction[j].getClassification()) {
                case 'I':
                    //leave the same
                    break;
                case 'A':
                    //check if address absolute excess machine size
                    if (address > machineSize) {
                        //change address into 0
                        address = 0;
                        error = " Error: Absolute address exceeds machine size; zero used.";
                    }
                    break;
                case 'R':
                    //check if relative address is bigger than module size
                    if (address >= vectorOfModule[i].vectorOfInstruction.size()) {
                        address = 0;
                        error = " Error: Relative address exceeds module size; zero used.";
                    }
                    else {
                        address += vectorOfModule[i].baseAddress; // add the base address
                    }
                    break;
                case 'E':
                    //determine which digit to select
                    int lastDigit = address%100;
                    
                    
                    //if last digit is out of range, treat as immediate
                    if (lastDigit > vectorOfModule[i].vectorOfSymbolUsed.size()) {
                        error = " Error: External address exceeds length of use list; treated as immediate.";
                    }
                    //if not use the value
                    else {
                        // which value to select
                        string variableSymbol = vectorOfModule[i].vectorOfSymbolUsed[lastDigit];
                        //get the value associated with the symbol
                        
                        //check if the value is defined
                        if (symbolsDefined.find(variableSymbol) != symbolsDefined.end()) {
                            address = symbolsDefined[variableSymbol];
                        }
                        else {
                            //if it was not defined, we will use 0 and give an error
                            address = 0;
                            error = " Error: " + variableSymbol + " is not defined; zero used.";
                        }
                    }
                    break;
            }
            int result = opcode * 1000 + address;
            cout << result << error << endl;
            counter ++;
        }
    }
    
    //error handlings
    
    for (int i = 0; i < vectorOfModule.size(); i++) {
        //defined but not used error
        //for each define
        for (auto symbolDefined =  vectorOfModule[i].mapOfSymbolDefined.begin(); symbolDefined !=  vectorOfModule[i].mapOfSymbolDefined.end(); symbolDefined++) {
            //check if the define is not in the set of use.
            if (setOfSymbolUsed.find(symbolDefined->first) == setOfSymbolUsed.end())
                cout << "Warning: " << symbolDefined->first << " was defined in module " << i << " but never used." << endl;
        }
        //appear in the use list but not used error
        
        bool symbolUsed = false;
        
        for (int k = 0; k < vectorOfModule[i].vectorOfSymbolUsed.size(); k++) {
            for (int j = 0; j < vectorOfModule[i].vectorOfInstruction.size(); j++) {
                if (vectorOfModule[i].vectorOfInstruction[j].getClassification() == 'E') symbolUsed = true;
            }
            if (!symbolUsed) {
                cout << "Warning: In module " << i << ", " << vectorOfModule[i].vectorOfSymbolUsed[k] << " appeared in the use list but was not actually used." << endl;
            }
        }
    }
    
    //definition exceed module size error
    
    for (auto definitionError = definitionModError.begin(); definitionError != definitionModError.end(); definitionError++ ) {
        cout << "Error: In module " << definitionError->second << " ,the def of " << definitionError->first << " exceeds the module size; zero (relative) used." << endl;
    }
    inputFile.close();
    return 0;
}

