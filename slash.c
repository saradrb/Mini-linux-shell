#include<readline/readline.h>
#include<readline/history.h>
#include <unistd.h>
#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#define MAX_ARGS_NUMBER 4096
#define MAX_ARGS_STRLEN 4096

int exit_()
{

    
    return 0;
}

int pwd()
{
    return 1;
}

int cd()
{
    return 2;
}

// function that takes the input line and parse it, it returns an array of args , number of args and the command   
char ** split_line(char* line,char** cmd,int *length,char* delimiters){

        char **list_arg = NULL;// allocate memory for the args array 
        *cmd = strtok(line,delimiters);  //the first arg is the command 
        int nb_spaces=0;
        //parse the line into args
        char* arg = strtok (NULL, " ");

        while(arg && (nb_spaces < MAX_ARGS_NUMBER)){
            nb_spaces++;
            list_arg = realloc (list_arg, sizeof (char*) *nb_spaces );
            if (list_arg == NULL)
                exit (-1); // memory allocation failed 
            list_arg[nb_spaces-1] = arg;
            arg = strtok (NULL, " ");
            
        }
        list_arg = realloc(list_arg, sizeof (char*) * (nb_spaces+1));
        list_arg[nb_spaces] = NULL;

        *length=nb_spaces;
        return list_arg;
}



// function that read the command line input and format it 
void read_cmd(){
    char* prompt = ">";
    int length = 0;
    char* cmd = " ";
    while(1){
        //read the command line input 
        char* line = readline(prompt);
        if (line && *line){// if the line isnt empty

            add_history(line);

            // split the line
            char** list_arg = split_line(line,&cmd,&length," ");
            
            // exec command 
            if (strcmp(cmd,"exit")==0){
                exit_();
            }else{
                if(strcmp(cmd,"cd")==0){
                    cd();
                }else{
                    if(strcmp(cmd,"pwd")==0){
                    pwd();
                    }else{
                        printf("%s: command not found\n",cmd);
                    }
                    }
            }

        //free the memory allocated
        free(list_arg);
        free(line);
        
        }
    }
    
}





int main(int argc, char const *argv[]) {
    //rl_outstream = stderr;
    read_cmd();
    return (0);
}
