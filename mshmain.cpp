/*
File: mshmain.cpp
Author: Kyle Shake
Date: 4/2/2018

Purpose: Calls main function for MYSHELL program

Main then uses the included files to call other 
program functions

*/

#include <iostream>
#include "global.h"

using namespace std;

int main(){

    initializeVariables();

    do{
        check_children(); 
        check_ShowTokens();
        cout << SHELL_PROMPT;
        scanner();        
    }while(1);

    return(0);
}
