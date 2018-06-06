/*
File: global.cpp
Author: Kyle Shake
Date: 4/2/2018

Purpose: Provides implementation for the 
global functions & variables declared in global.h
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <cstring>
#include "global.h"



using namespace std;

vector <string> SH_COMMANDS {"setvar", "setprompt", "setdir", "showprocs", "done", "run", "fly", "tovar"};

map <string, string> SH_VARIABLES;

map <int, string> SH_PROCESSES;

string SHELL_PROMPT = "msh > ";

/* Adds reserved varnames and their values to Variables map */
void initializeVariables(){
    SH_VARIABLES["PATH"] = "/bin:/usr/bin";
    SH_VARIABLES["ShowTokens"] = "0";
    return;
}

/*Function checks if any child processes 
have completed. If so, print which one*/
void check_children(){
    int child_process = waitpid(-1, 0, WNOHANG);
    if(child_process == -1){
//        cout << "No child processes running" << endl; //Debug
    }else if(child_process == 0){
//        cout << "Child Processes running but no child processes completed" << endl;
    }else{
        cout << "Completed: " << SH_PROCESSES[child_process] << endl;
        SH_PROCESSES.erase(child_process);
    }
    return;
}

/*Function checks to see if the ShowTokens
variable was changed to a 1. If so, print variables */
void check_ShowTokens(){
    if(SH_VARIABLES["ShowTokens"] == "1"){
        map<string, string>::iterator it;

        for(it = SH_VARIABLES.begin(); it != SH_VARIABLES.end(); it++){
            cout <<  it->first << " = " << it->second << endl;
        }
    }else{
     //do nothing
    }
}

void removeCharsFromStr(string &str, char* charsToRemove){
    for(int i=0; i < strlen(charsToRemove); ++i){
        str.erase( remove(str.begin(), str.end(), charsToRemove[i]), str.end() );
    }
}

/* Function changes the prompt for msh */
void setNewPrompt(string input, vector<string>&v){
    if(v.size() > 1){
        removeCharsFromStr(v[1], "\"");
        SHELL_PROMPT = v[1];   
    }else{
        if(input == ""){
            cout << "Command: setprompt must have token!" << endl;
            cout << "Usage: setprompt <Prompt>" << endl;
            return;
        }else{
            SHELL_PROMPT = input;
        }
    }
    return;
}//End of setprompt()


/* Function checks if string is a positive integer */
bool isPosInt(string possInt){
    if(possInt.empty() || ((!isdigit(possInt[0])) && (possInt[0] != '+')) || (possInt[0] == '-')) return false;

    char *p;
    strtol(possInt.c_str(), &p, 10);

    return (*p == 0);
}

