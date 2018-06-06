/*
File: mshshell.cpp
Date: 3/29/2018
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

#define MAX_CHAR_LEN 256
#define BLOCK_SIZE 512

using namespace std;

/*GLOBALS*/
vector <string> SH_COMMANDS {"setvar", "setprompt", "setdir", "showprocs", "done", "run", "fly", "tovar"};

map <string, string> SH_VARIABLES;

map <int, string> SH_PROCESSES;

string SHELL_PROMPT = "msh > ";
/*END GLOBALS*/

/*Function declarations */
void parser(vector<string>&v);
void setNewPrompt(string prompt);

/* Adds reserved varnames and their values to Variables map */
void initializeVariables(){
    SH_VARIABLES["PATH"] = "/bin:/usr/bin";
    SH_VARIABLES["ShowTokens"] = "0";
    return;
}

/* Checks if token is a variable type */
bool isVarType(string varname){
    if(varname.at(0) == '^'){
        return true;
    }else{
        return false;
    }
}

/* Checks if variable has been assigned */
bool VarisAssigned(string varname){
    if((SH_VARIABLES.find(varname))!=SH_VARIABLES.end()){
        return true;
    }else{
        return false;
    }
}


/* Function replaces variable name with variable value */
string replaceVarWithValue(string varname){
    string value = SH_VARIABLES.find(varname)->second;
    return value;
}


/* Function checks if string is a positive integer */
bool isPosInt(string possInt){
    if(possInt.empty() || ((!isdigit(possInt[0])) && (possInt[0] != '+')) || (possInt[0] == '-')) return false;

    char *p;
    strtol(possInt.c_str(), &p, 10);

    return (*p == 0);
}

/* Function removes remaining whitespace from tokens
if token is completely whitespace or blank, token is erased */
void remove_whitespaceTokens(vector <string> &v){
    size_t pos;
    for(int i = 0; i < v.size(); i++){
        if(v[i] == ""){
            v.erase(v.begin()+i);
        }
        if((pos = v[i].find_first_not_of(' ')) == string::npos){
            v.erase(v.begin()+i);
        }else{
            string substr = v[i].substr(pos, string::npos);
            v[i] = substr;
        }
    }
}


/* Function pulls input, scans it for proper syntax, and tokenizes it */
void scanner(){
    string input;
    getline(cin, input); 

    /* Ctrl+D check */
    if(cin.eof())	exit(0);

    /* Checking for comments and if found, remove from input*/
    string commentdelim = "#";
    string comment;
    size_t pos = 0;
    if((pos = input.find(commentdelim)) != string::npos){
        comment = input.substr(pos, string::npos);
//        cout << "Comment: " << comment << endl; //Debug

        input = input.substr(0, pos-1);
    }else{
//        cout << "No comments found!" << endl; //Debug
    }


    /* Split input string into tokens by quotation marks first
       then split remaining tokens by whitespace */
    pos = 0;
    string delim = "\"";
    string token;
    vector <string> tokens;

    while((pos = input.find(delim)) != string::npos){
        token = input.substr(0,pos);
        input = input.substr(pos+1, string::npos);
        tokens.push_back(token);
    } 

    pos = 0;
    delim = " ";
    vector <string> temp;

    if(tokens.size() == 0){
        while((pos = input.find(delim)) != string::npos){
            token = input.substr(0,pos);
            if(token == "setprompt"){
                string prompt = input.substr(pos+1, string::npos);
                setNewPrompt(prompt);
                return;                    
            }

            input = input.substr(pos+1, string::npos);
            tokens.push_back(token);
        }
        tokens.push_back(input);
    }else{
        for(int i = 0; i < tokens.size(); i++){
            while((pos = tokens[i].find(delim)) != string::npos){
                token = tokens[i].substr(0, pos);
                if(token.at(0) == '\"'){
                    token = token.substr(1, token.size()-1);
                    temp.push_back(token);
                    continue;
                }else{
                    tokens[i] = tokens[i].substr(pos+1, string::npos);
                    temp.push_back(token);
                }
            }
            if(pos == string::npos && tokens[i] != ""){
                temp.push_back(tokens[i]);
            }
        }

        tokens.clear();
        tokens = temp;
    }

    /* Command to msh should be first token
       if it's a space, erase it and try the next one
    */
    string command = tokens[0];
    while(command == " "){
        tokens.erase(tokens.begin());
        command = tokens[0];
    }

    /* setprompt command uses the whitespaces after
     the first space but if it's not setprompt,
     we don't need the whitespace tokens
    */
    
    if(command != "setprompt"){
        remove_whitespaceTokens(tokens);
    }else{
        if(tokens.size() > 1){
            string prompt = tokens[1];
            setNewPrompt(prompt);
            return;
        }else{
            cout << "Command: setprompt must have token!" << endl;
            cout << "Usage: setprompt <Prompt>" << endl;
            return;
        }
    }


    /* check for variable names and replace values
    */
    pos = 0;
    for(int i = 0; i < tokens.size(); i++){
        if(isVarType(tokens[i])){
            string varname = tokens[i].substr(1, string::npos);
            if(VarisAssigned(varname)){
                tokens[i] = replaceVarWithValue(varname);
            }else{
                cout << "Variable: " << varname << " has not yet been assigned!" << endl; //debug
            }
        }
    }

    /* Compare first token to list of available commands 
    */
    if(find(begin(SH_COMMANDS), end(SH_COMMANDS), command) != end(SH_COMMANDS)){
        parser(tokens);
        return;
    }else{
        cout << "-msh: " << command << ": command not found" << endl;
        return;   
    }
}//End of scanner()


/* Function changes the prompt for msh */
void setNewPrompt(string prompt){
    size_t pos = 0;
    if(prompt ==""){
        cout << "New prompt not designated!" << endl;
        cout << "Usage: setprompt <NewPrompt>." << endl;
        return;
    }
    
    if((pos = prompt.find_first_not_of(" ")) == string::npos){
        
        cout<< "New prompt cannot be just whitespace!" << endl;
        return;
    }
    SHELL_PROMPT = prompt;
}//End of setprompt()



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
    if((cmd[0] == '/') || ((cmd.substr(0,1)) == "./") || (isalpha(cmd[0])))	return true;
    else return false;
}


/* Checks to make sure directory is
absolute( starts with '/'), or relative (start with alphabetic char) */
bool validdirectory(string dir){
    if((dir[0] == '/') || (isalpha(dir[0])))	return true;
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
void parser(vector<string> &v){
    string command = v[0];

    /* Match the command and execute */
    if(command == "setvar"){
        if(v.size() == 3){
            setvar(v[1], v[2]);
        }else if(v.size() > 3){
            cout << "Too many arguments for setvar!" << endl;
            cout << "Usage: setvar <Variable Name> <Value>" << endl;
            return;
        }else{
            cout << "Too few arguments for setvar!" << endl;
            cout << "Usage: setvar <Variable Name> <Value>" << endl;
            return;
        }
    }else if(command == "setprompt"){
        if(v.size() == 2){
            cout << "Should never reach this point!";
            cout << " Prompt should have already been set!" << endl;
            setNewPrompt(v[2]);
        }else if(v.size() > 2){
            cout << "Too many arguments for setprompt!" << endl;
            cout << "Usage: setprompt <Prompt>" << endl;
            return;
        }else{
            cout << "Too few arguments for setprompt!" << endl;
            cout << "Usage: setprompt <Prompt>" << endl;
            return;
        }
    }else if(command == "setdir"){
        if(v.size() == 2){
            if(validdirectory(v[1])){
                setdir(v[1]);
                return;
            }else{
                cout << "Directory: " << v[1] << " not a valid directory!" << endl;
                return;
            }
        }else if(v.size() > 2){
            cout << "Too many arguments to setdir!" << endl;
            cout << "Usage: setdir <PATH>" << endl;
            return;
        }
    }else if(command == "showprocs"){
        if(v.size() == 1){
           showprocs();
        }else{
           cout << "Command: showprocs does not take arguments!" << endl;
           return;
        }
    }else if(command == "done"){
        if(v.size() == 1){
            exit(0);
        }else if(v.size() == 2){
            if(isPosInt(v[1])){
                char *end;
                long i = strtol(convert(v[1]), &end, 10);
                exit(i);
            }else{
                cout << "Incorrect argument: " << v[1] << ". Must be non-negative integer" << endl;
                return;
            }
        }else{
            cout << "Too many arguments for done!" << endl;
            cout << "Usage: done <value>" << endl;
            return;
        }
    }else if(command == "run"){
        if(validcommand(v[1])){
            string cmdpath = whatPath(v[1]);
            v.erase(v.begin()); //Erase "run"
            runcmd(cmdpath, v);
            return;
        }else{
            cout << "Argument: " << v[1] << " was not valid!" << endl;
            return;
        }
    }else if(command == "fly"){
        if(validcommand(v[1])){
            string cmdpath = whatPath(v[1]);
            v.erase(v.begin()); //Erase "fly"
            flycmd(cmdpath, v);
            return;
        }else{
            cout << "Argument: " << v[1] << " was not valid!" << endl;
            return;
        }
    }else if(command == "tovar"){
        string varname = v[1];
        if(validcommand(v[2])){
            string cmdpath = whatPath(v[2]);
            v.erase(v.begin(), v.begin()+1); //Erase "tovar" and "<variable>"
            tovarcmd(varname, cmdpath, v);
            return;
        }else{
            cout << "Argument: " << v[2] << " was not valid!" << endl;
            return;
        }
    }else{
        cout << "I don't know how that snuck by me" << endl; //Debug

    }
}

/*Function checks to see if any child processes
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
