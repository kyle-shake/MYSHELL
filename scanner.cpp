/*
File: scanner.cpp
Author: Kyle Shake
Date: 4/2/18

Purpose:
    Provides the functions for scanning the input of the MYSHELL program

Working On:

4/4 @ 8:51am
Line 120: Parsing command out of string in tokens vector after
input was tokenized at quotation marks. 
--Don't know what to do with the other substr 
	once the command has been removed.
--Ideas: Move command, other substr, and quotestr to temp vector?


*/

#include <sys/types.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include "global.h"


using namespace std;

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
/*Function removes any spaces that user may
type before command */
void trimWSfromfront(string &input){
    size_t pos = 0;
    string whitespace = " \t";
    pos = input.find_first_not_of(whitespace);
    input = input.substr(pos, string::npos);
}

/*Function removes comments from the input so
we don't tokenize them */
void stripComments(string &input){

    string delim = "#";
    string comment;
    size_t pos = 0;
    if((pos = input.find(delim)) != string::npos){
        comment = input.substr(pos, string::npos);
//        cout << "Comment: " << comment << endl; //Debug

        input = input.substr(0, pos-1);
    }else{
//        cout << "No comments found!" << endl; //Debug
    }
}



/*Function takes the user input and parses the shell command
  from it*/
string isolateCommand(string &input){

    /*Find first space and split the string at space
      the first token should be the command*/
    string command;
    size_t pos;
    string delim = " ";
    pos = input.find(delim);

    if(pos == string::npos){//If there is not a space, the input is the command
        command = input;
    }else{ //If there is a space, split the input at the space
        command = input.substr(0, pos);
        input = input.substr(pos, string::npos);
    }
    
    return command;
}//end of isolateCommand()


/*Function takes input after command has been removed 
and tokenizes it at quotation marks  */
void tokenizeByQuotes(string &input, vector<string>&tokens){
    size_t pos = 0;
    string delim = "\"";
    string token;
    string quotestr;
    while((pos = input.find(delim, pos)) != string::npos){
            token = input.substr(0,pos-1);
            size_t pos2 = pos + 1;
            pos2 = input.find(delim, pos2);
            quotestr = input.substr(pos, pos2); 
            tokens.push_back(token);
            tokens.push_back(quotestr);
            pos = 1;
       if(pos2 != string::npos){
            input = input.substr(pos2+1, string::npos);
       }else{
            break;
       }
   }
}//end of tokenizeByQuotes()


/* Function takes input after command has been removed
and it has been (potentially) tokenized by quote marks
then tokenizes it (or the tokens from quotes) at the whitespaces */
void tokenizeBySpaces(string input, vector<string> &tokens){

    size_t pos = 0;
    string token;
    string delim = " ";

    vector <string> temp;

    if(tokens.size() > 1){ /*if input has already been 
				tokenized by quotation marks*/

    /*Attempt to tokenize the individual tokens*/
        for(int i = 0; i < tokens.size(); i++){

            /*Trim whitespace from front of token*/
            trimWSfromfront(tokens[i]);

            /*Don't split tokens that start with quote marks*/
            if(tokens[i].at(0) == '\"'){ 
                temp.push_back(tokens[i]);
                continue;
            }else{

            /*Otherwise split each token by whitespace and add to temp vector*/
                while((pos = tokens[i].find(delim, pos)) != string::npos){
            
                    token = tokens[i].substr(0, pos);
                    temp.push_back(token);
                    tokens[i] = tokens[i].substr(pos+1, string::npos);
                    trimWSfromfront(tokens[i]);
                    pos = 0;
                }
                if(tokens[i] != ""){ /*Can't forget to add the last substr if it exists*/
                    temp.push_back(tokens[i]);
                }
         
            }

        }//end for
        
        if(input != ""){/*Also can't forget the remaining input
			**One such case: Quote string fell in middle of input*/

        /*Trim whitespace from front of input*/
            trimWSfromfront(input);

        /*Try to split by whitespace & add to temp vector*/
            pos = 0;
            while((pos = input.find(delim, pos)) != string::npos){
                token = input.substr(0, pos);
                temp.push_back(token);
                input = input.substr(pos+1, string::npos);
                trimWSfromfront(input);
                pos = 0;
            }
            if(pos == string::npos && input != ""){/*Can't forget to add the last part of input
							if it exists or didn't have whitespace*/
               temp.push_back(input);
            }
        }
    }else{/*if input was not tokenized by quotation marks*/

    /*Trim whitepsace from front of input*/
        trimWSfromfront(input);

    /*Split input by spaces
      At this point, we *shouldn't* have to worry about quote marks
      Whitespace is our only delimiter
    */
        while((pos = input.find(delim, pos)) != string::npos){
            token = input.substr(0,pos);
            temp.push_back(token);
            input = input.substr(pos, string::npos);
            trimWSfromfront(input);
            pos = 0;
        }
        if(pos == string::npos && input != ""){/*Like before, don't forget last part of input
						 if it exists or didn't have whitespace*/
        temp.push_back(input);
        }

    } 

    /*Clear the old tokens vector and replace it with the fully tokenized temp vector*/
    tokens.clear();
    tokens = temp;

}//end tokenizeBySpaces()

/*Function iterates through tokens looking for 
variable names */
void replaceVariables(vector<string>&tokens){

    size_t pos = 0;
    string varname;

    /*Loop through tokens to see if any start with variable demarcation ("^")*/
    for(int i = 0; i < tokens.size(); i++){
        if(isVarType(tokens[i])){
            varname = tokens[i].substr(1, string::npos);
            if(VarisAssigned(varname)){
                tokens[i] = replaceVarWithValue(varname);
            }else{
                cout << "Variable: " << varname << " has not yet been assigned!" << endl; //debug
            }
        }

        /*However, quote string tokens won't start with variable demarcation
          but they may contain variables

         Thus, we must look through the characters of quote strings */
        if(tokens[i].at(0) == '\"'){
            string delim = "^";
            string QuoteAndWSdelim = " \"\t\f\v\n\r";
            pos = 0;
            if((pos = tokens[i].find(delim, pos)) != string::npos){
                size_t pos2 = tokens[i].find_first_of(QuoteAndWSdelim, pos+1);
                varname = tokens[i].substr(pos, pos2-pos);
                if(isVarType(varname)){
                    varname = varname.substr(1, string::npos); //Remove the ^ to check against Variable Map
                    if(VarisAssigned(varname)){
                        string varvalue = replaceVarWithValue(varname);
                        tokens[i].replace(pos, varname.length()+1, varvalue); 
                    }else{
                        cout << "Variable: " << varname << " has not yet been assigned!" << endl; //debug
                    }
                }else{
                    cout << "Something went wrong! ";
                    cout << varname << " is not variable type?" << endl;
                }
            }
        }


    }//end for
}

/* Function removes quote marks from quote tokens
in order to pass to command*/
void removeQuoteMarks(vector<string>&v){

    int lastpos;
    for(int i = 0; i < v.size(); i++){
        lastpos = v[i].size() - 1;

        if(v[i].at(0) == '\"' && v[i].at(lastpos) == '\"'){
            v[i] = v[i].substr(1, lastpos-1);
        }
    }
}

/* Main Driver Function of Scanner

Pulls input
Scans it for MSH proper syntax
Tokenizes it 

*/
void scanner(){
    string input;
    getline(cin, input);
    vector <string> tokens;

    /* Ctrl+D check */
    if(cin.eof())       exit(0);

    /*Trim whitespace from beginning of user input */
    trimWSfromfront(input);

    /* Checking for comments and if found, remove from input*/
    stripComments(input);    

    /*isolate the command*/
    string command = isolateCommand(input);

    /* Split input string into tokens by quotation marks first
       then split remaining tokens by whitespace */
    tokenizeByQuotes(input, tokens); 

    /*setprompt is a finnicky command that accepts whitespace
      lets deal with it here before parsing the input by whitespace*/
    if(command == "setprompt"){
        setNewPrompt(input, tokens);
        return;
    }

    /* Split remaining input into tokens at spaces */
    tokenizeBySpaces(input, tokens);

    /* 
     we don't need the whitespace tokens
    */
    remove_whitespaceTokens(tokens);

    /* check for variable names and replace values
    */
    replaceVariables(tokens);

    /*Since we are finished using quote marks for scanning,
      we need to now remove them.
    */
    
    removeQuoteMarks(tokens);

    /* Compare extracted command to list of available commands 
    */
    if(find(begin(SH_COMMANDS), end(SH_COMMANDS), command) != end(SH_COMMANDS)){
        parser(command, tokens);
        return;
    }else{
        cout << "-msh: " << command << ": command not found" << endl;
        return;
    }
}//End of scanner()

