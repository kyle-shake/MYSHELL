/*
File: global.h
Date: 4/2/18
Author: Kyle Shake

Purpose: Provide the implementation for 
the necessary global functions, variables and definitions

*/

#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <map>
#include <algorithm>
#include <string>

#define MAX_CHAR_LEN 256
#define BLOCK_SIZE 512

using namespace std;

/*GLOBAL VARIABLES*/
extern vector <string> SH_COMMANDS;

extern map <string, string> SH_VARIABLES;

extern map <int, string> SH_PROCESSES;

extern string SHELL_PROMPT;
/*END GLOBAL VARIABLES*/

/*GLOBAL FUNCTIONS*/
void initializeVariables();

void check_children();

void check_ShowTokens();

void removeCharsFromStr(string &str, char* chars);

void setNewPrompt(string s, vector<string>&v);

bool isPosInt(string s);
/*END GLOBAL FUNCTIONS*/

/*SCANNER FUNCTIONS*/

void scanner();

/*END SCANNER FUNCTIONS*/

/*PARSER FUNCTIONS*/

void parser(string s, vector<string>&v);

/*END PARSER FUNCTIONS*/

#endif
