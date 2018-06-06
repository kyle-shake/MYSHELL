/*
File: parser.cpp
Date: 4/2/18
Author: Kyle Shake

*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <map>
#include <algorithm>
#include <string.h>
#include <wait.h>
#include <errno.h>
#include "global.h"

using namespace std;

/* Function stores variable and its value in the global map */
void setvar(string varname, string value){
/*    if(VarisAssigned(varname)){
        cout << "Overwriting " << varname << endl; //Debug
    }else{
        cout << "Variable created: " << varname << " with value: " << value << endl; //Debug
    }*/
    SH_VARIABLES[varname] = value;
}//End of setvar()

/* Function gets current working directory then returns it as a string */
string getCWDir(){
    const size_t dirLen = 255;
    char dirBuffer[dirLen];
    if(getcwd(dirBuffer, sizeof(dirBuffer)) != NULL){
        return dirBuffer;
    }
    //May add something incase buffer size is too small

}//End of getCWDir()


/*Function sets the current directory */
void setdir(string dirname){

    string currdir = getCWDir();
    cout << "Current Working Directory is " << currdir << endl; //Debug

    string newdir = dirname;
    int res = chdir(newdir.c_str());

    /*


INSERT ERROR HANDLING


    */
    currdir = getCWDir();
    cout << "New Working Directory is " << currdir << endl; //Debug: For verifying directory change
}//End of setdir()

/*Function shows the list of current processes running */
void showprocs(){

    cout << "Current processes:" << endl;
    map<int, string>::iterator it;

    for(it = SH_PROCESSES.begin(); it != SH_PROCESSES.end(); it++){
        cout << "PID: " << it->first << "Process: " << it->second << endl;
    }
}


/* Checks to make sure command to run starts with
a '/', a './' or is alphabetic */
bool validcommand(string cmd){
    if((cmd[0] == '/') || ((cmd.substr(0,1)) == "./") || (isalpha(cmd[0])))     return true;
    else return false;
}


/* Checks to make sure directory is
absolute( starts with '/'), or relative (start with alphabetic char) */
bool validdirectory(string dir){
    if((dir[0] == '/') || (isalpha(dir[0])))    return true;
    else return false;
}



/* Function determines what path to run the Unix command */
string whatPath(string path){

    string cmdpath;
    if((path.substr(0,1)) == "./"){
        cmdpath = getCWDir();
    }else if(path[0] == '/'){
        cmdpath = path;
    }else if(isalpha(path[0])){
        cmdpath = SH_VARIABLES["PATH"];
    }

    return cmdpath;
}

char *convert(const string &s){
    char *buf = new char[s.size()+1];
    strcpy(buf, s.c_str());
    return buf;
}

/*Function creates a child and runs a user specified command 
with that child. Waits for child to complete before continuing
with parent process */
void runcmd(string path, vector<string>vofargs){

    /* Converting strings to C Strings and
    then putting them into C String arrays for execv arguement*/
    int num_of_strings = vofargs.size();
    char *args[num_of_strings];
    for(int i = 0; i < vofargs.size(); i++){
        args[i] = convert(vofargs[i]);
    }
    args[num_of_strings] = (char *) 0;

    //Start fork and assign a PID 
    pid_t pid;
    pid = fork();
    int child_status;

    if(pid == 0){/*child*/
//        cout << "The child is running" << endl; //Debug

        if(path == SH_VARIABLES["PATH"]){
            execvp(args[0], args);
        }else if (path[0] == '/'){
            execv(path.c_str(), args);
        }else{
            char* home = "HOME=";
            strcat(home, path.c_str());

            char *env[2];
            env[0] = home;

            execve(args[0], args, env);
        }

        /* These lines should not be executed */
        cout << "The child failed to exec the program." << endl; //For Debugging
        exit(1);

    }else{ /*parent*/
//        cout << "The parent created a child to run..." << endl; //For Debugging

        child_status = waitpid(pid, 0, 0); /*block parent until child finishes*/

        /* For Debugging 
        if(WIFEXITED(child_status) && !WEXITSTATUS(child_status)){
            cout << "Child completed process successfully" << endl;

        }else{
            cout << "Child exited abnormally" << endl;
        }*/
    }

    return;
}//End of runcmd()


/*
Function creates a child and runs a user specified command
with that child. DOES NOT wait for child to complete before 
continuing with parent process */
void flycmd(string path, vector<string>vofargs){
    int num_of_strings = vofargs.size();
    char *args[num_of_strings];
    string process ="";
    for(int i = 0; i < vofargs.size(); i++){
        args[i] = convert(vofargs[i]);
        process += vofargs[i] + " ";
    }
    args[num_of_strings] = (char *) 0;

    pid_t pid;
    pid = fork();

    SH_PROCESSES[pid] = process;
    if(pid == 0){ /*child*/
//        cout << "The child is running " << args[0] << endl; //Debug

        if(path == SH_VARIABLES["PATH"]){
            execvp(args[0], args);
        }else if (path[0] == '/'){
            execv(path.c_str(), args);
        }else{
            char* home = "HOME=";
            strcat(home, path.c_str());

            char *env[2];
            env[0] = home;

            execve(args[0], args, env);
        }

        /* These lines should not be executed */
        printf("The child failed to exec the program.\n");
        exit(1);
    }else{ /* parent */
        return;
    }
}//End of flycmd()


/*
Function creates a child and runs a user specified command 
with that child. Waits for child to complete process and then 
assigns the output of process to a variable using pipe */
void tovarcmd(string varname, string path, vector<string>vofargs){
     if(vofargs[0] == varname){
         vofargs.erase(vofargs.begin());
     }

     int num_of_strings = vofargs.size();

     string strargs = "";
     for(int i = 0; i < vofargs.size(); i++){
        strargs += vofargs[i];
        strargs += " ";
     }
     char args[strargs.length()+1];

     strcpy(args, strargs.c_str());

     FILE* pfile = popen(args, "r");
     char buffer[BLOCK_SIZE];
     string value = "";

     while(fgets(buffer, sizeof(buffer), pfile) != NULL){
         value += buffer;
     }
     pclose(pfile);
     setvar(varname, buffer);
     cout << "Output captured: " << value << endl;
     return;
}



/* Function checks tokenized input and handles the commands accordingly */
void parser(string command, vector<string> &v){


    /* Match the command and execute */
    if(command == "setvar"){
        if(v.size() == 2){
            setvar(v[0], v[1]);
        }else if(v.size() > 2){
            cout << "Too many arguments for setvar!" << endl;
            cout << "Usage: setvar <Variable Name> <Value>" << endl;
            return;
        }else{
            cout << "Too few arguments for setvar!" << endl;
            cout << "Usage: setvar <Variable Name> <Value>" << endl;
            return;
        }
    }else if(command == "setdir"){
        if(v.size() == 1){
            if(validdirectory(v[0])){
                setdir(v[0]);
                return;
            }else{
                cout << "Directory: " << v[0] << " not a valid directory!" << endl;
                return;
            }
        }else if(v.size() > 1){
            cout << "Too many arguments to setdir!" << endl;
            cout << "Usage: setdir <PATH>" << endl;
            return;
        }
    }else if(command == "showprocs"){
        if(v.size() == 0){
           showprocs();
        }else{
           cout << "Command: showprocs does not take arguments!" << endl;
           return;
        }
   }else if(command == "done"){
        if(v.size() == 0){
            exit(0);
        }else if(v.size() == 1){
            if(isPosInt(v[0])){
                char *end;
                long i = strtol(convert(v[0]), &end, 10);
                exit(i);
            }else{
                cout << "Incorrect argument: " << v[0] << ". Must be non-negative integer" << endl;
                return;
            }
        }else{
            cout << "Too many arguments for done!" << endl;
            cout << "Usage: done <value>" << endl;
            return;
        }
    }else if(command == "run"){
        if(validcommand(v[0])){
            string cmdpath = whatPath(v[0]);
            runcmd(cmdpath, v);
            return;
        }else{
            cout << "Argument: " << v[1] << " was not valid!" << endl;
            return;
        }
    }else if(command == "fly"){
        if(validcommand(v[0])){
            string cmdpath = whatPath(v[0]);
            flycmd(cmdpath, v);
            return;
        }else{
            cout << "Argument: " << v[1] << " was not valid!" << endl;
            return;
        }
    }else if(command == "tovar"){
        string varname = v[0];
        if(validcommand(v[1])){
            string cmdpath = whatPath(v[1]);
            tovarcmd(varname, cmdpath, v);
            return;
        }else{
            cout << "Argument: " << v[1] << " was not valid!" << endl;
            return;
        }
    }else{
        cout << "I don't know how that snuck by me" << endl; //Debug

    }
}//end of parser()





