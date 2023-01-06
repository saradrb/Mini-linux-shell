#include <readline/history.h>
#include <readline/readline.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>
#include "library/string.c"
#include "internal_commands.h"
#include "signal.h"

extern char previous_rep[PATH_MAX];
extern char current_rep[PATH_MAX];
extern int previous_return_value;

/**
 * @brief Execute every external command with or with out argument
 *
 * @param cmd command
 * @param args array containing the command and his arguments
 * @return the return value of the execution of the command : 0 for successed, 1
 * if exec failed, 8 if exec successed but the command or argument is wrong
 */
static int extern_command(char *cmd, char **args) {
  int status = 0;
  int return_value = 0;
  // Create child process
  pid_t pid_fork = fork();
  switch (pid_fork) {
    // If the fork has failed
    case -1:
      write(STDOUT_FILENO, "Error fork\n", 12);
      return 1;
    case 0:
      // Execute the command : if the execution fail, exit with a specific
      // value
      treat_signal(false);
      execvp(cmd, args);
      exit(127);
    default:
      // wait for the end of child process and take his exit value
      waitpid(pid_fork, &status, 0);
      if (WIFSIGNALED(status)) {
        return_value = 255;
      } else if (WIFEXITED(status)) {
        return_value = WEXITSTATUS(status);
      }
      // if the execution of the command has failed
      return return_value;
  }
}


int handle_redirection(char* cmd,char ** args, char* redirection ,char* filename){
  int input_fd=0,output_fd=0;
  int result;
  if (strcmp(redirection,"<")==0) { //redirecting stdin of the command to the file filename 

    input_fd= open(filename,O_RDONLY);
    if (input_fd == -1){perror("opening file error"); return 1;}

    result=dup2(input_fd,STDIN_FILENO);
    if ( result == -1){perror("error redirecting stdin"); return 1;}

    extern_command(cmd,args);
    
    close(input_fd);
  
  }else {
    if (strcmp(redirection,">")==0 ){
        output_fd = open(filename, O_WRONLY|O_CREAT|O_EXCL, 0644);
        if (input_fd == -1){perror("opening file error"); return 1;}

        result=dup2(output_fd,STDOUT_FILENO);
        if ( result == -1){perror("error redirecting stdout"); return 1;}

        extern_command(cmd,args);
        
        close(output_fd);
    }else {
      if (strcmp(redirection,">|")==0){

        output_fd = open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0644);
        if (input_fd == -1){perror("opening file error"); return 1;}

        result=dup2(output_fd,STDOUT_FILENO);
        if ( result == -1){perror("error redirecting stdout"); return 1;}

        extern_command(cmd,args);
        
        close(output_fd);
      }else {
      
        if (strcmp(redirection,">>")==0){

          output_fd = open(filename, O_WRONLY|O_CREAT|O_APPEND, 0644);
          if (input_fd == -1){perror("opening file error"); return 1;}

          result=dup2(output_fd,STDOUT_FILENO);
          if ( result == -1){perror("error redirecting stdout"); return 1;}

          extern_command(cmd,args);
            
          close(output_fd);

        }else{
          if (strcmp(redirection,"2>")==0 || strcmp(redirection,"2>|")==0 || strcmp(redirection,"2>>")==0 ){
            if (strcmp(redirection,"2>")==0) {

              output_fd = open(filename, O_WRONLY|O_CREAT|O_EXCL, 0644);
              
            }else{

              if(strcmp(redirection,"2>|")==0 ){

                output_fd = open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0644);

              }else{

                output_fd = open(filename, O_WRONLY|O_CREAT|O_APPEND, 0644);

              }

            }

            if (output_fd == -1){perror("opening file error"); return 1;}
            
            result=dup2(output_fd,STDERR_FILENO);
            if ( result == -1){perror("error redirecting stderr"); return 1;}

            extern_command(cmd,args);
            
            close(output_fd);
          
          } else return 1;
        }
      } 
    }
  }
  return 0;
}


/**
 * @brief Create an array with the command followed by list_arg
 *
 * @param length length of list_arg
 * @param list_arg list of arguments
 * @param cmd the command
 * @return the fianl array or NULL
 */
// static char **array_with_cmd_name(int length, char **list_arg, char *cmd) {
//   // allocate the memory
//   char **array = malloc(sizeof(char *) * (length + 2));
//   if (array == NULL) {
//     return NULL;
//   }
//   // fill the array
//   array[0] = cmd;
//   for (int i = 1; i <= length + 1; i++) {
//     array[i] = list_arg[i - 1];
//   }
//   return array;
// }

/**
 * @brief Print prompt with color, containing the return value of the last
 * executed command and the path where we are
 */
static void my_prompt() {
  // convert the previous return value into a char[]
  char nbr[6] = {'\0'};
  if (previous_return_value == 255) strncpy(nbr, "SIG", 4);
  else sprintf(nbr, "%d", previous_return_value);

  char res[50] = {'\0'};
  // complete res with green if the last return value is 0, and with red
  // otherwise
  if (previous_return_value == 0) {
    char *start_prompt_green = "\001\033[32m\002[";
    strcat(res, start_prompt_green);
  } else {
    char *start_prompt_red = "\001\033[91m\002[";
    strcat(res, start_prompt_red);
  }
  strcat(res, nbr);

  // change the color to cyan
  char *start_pwd_cyan = "]\001\033[36m\002";
  strcat(res, start_pwd_cyan);

  char *end_prompt = "\001\033[00m\002";
  int len = strlen(current_rep);
  // complete res with the absolute path, or with a simplification of the
  // absolute path if length of path is more then 26 characters
  if (len > 26 - strlen(nbr)) {
    strcat(res, "...");
    strcat(res, &current_rep[len - 23 + strlen(nbr)]);
  } else {
    strcat(res, current_rep);
  }
  // change the color to default and write res into STDERR
  strcat(res, end_prompt);
  if (write(STDERR_FILENO, res, strlen(res)) != strlen(res)) {
    return;
  }
}


// function that takes the input line and parse it, it returns an array of args
// , number of args and the command
static char **split_line(char *line, char **cmd, int *length,
                         char *delimiters) {
  char **list_arg = NULL;           // allocate memory for the args array
  *cmd = strtok(line, delimiters);  // the first arg is the command
  int nb_spaces = 0;
  // parse the line into args
  char *arg = strtok(NULL, " ");

  while (arg && (nb_spaces < MAX_ARGS_NUMBER)) {
    nb_spaces++;
    list_arg = realloc(list_arg, sizeof(char *) * nb_spaces);
    if (list_arg == NULL) exit(-1);  // memory allocation failed
    list_arg[nb_spaces - 1] = arg;
    arg = strtok(NULL, " ");
  }
  list_arg = realloc(list_arg, sizeof(char *) * (nb_spaces + 1));
  list_arg[nb_spaces] = NULL;

  *length = nb_spaces;
  return list_arg;
}

/**
 * @brief function that read the command line input and format it
 */
static void read_cmd() {
  char *prompt_char = "$ ";
  int length = 0;
  char *cmd = " ";
  int nb_parts=0;

  while (1) {
    my_prompt();
    // read the command line input
    char *line = readline(prompt_char);
    char *trimmed_line = trim(line);

    // if the file has ended, exit the program
    if (trimmed_line == NULL) {
      write(STDOUT_FILENO, "\n", 1);
      my_exit(NULL, 0);
    }

    // if the line isnt empty
    if (trimmed_line && *trimmed_line) {
      add_history(trimmed_line);

      // split the line
      char **list_arg = split_line(trimmed_line, &cmd, &length, " ");
        //test if there is an argument that contains a *
            int size=0;
            char** args_extanded=NULL;
            int nb_cmds=0;
            if(strstr(cmd,"*")){
              char* expanded_path=malloc(sizeof(char)*PATH_MAX);
              memset(expanded_path,0,sizeof(char)*PATH_MAX);
              char**mypath=parse_path(cmd,&nb_parts,"/");
              char ** new_cmd = malloc(sizeof(char*)*10);
              expand_star(mypath,nb_parts,expanded_path,new_cmd,&nb_cmds);
              free(mypath);
              free(expanded_path);
              
              if(nb_cmds){
                args_extanded=concat_tab(args_extanded,&size,new_cmd,nb_cmds);
                cmd=args_extanded[0];
                }
              free(new_cmd);
              
              }
              else{args_extanded=concat_elem(args_extanded,&size,cmd);}
            
            for (int i = 0; i < length; i++)
            {
              if(strstr(list_arg[i],"*")){
                
              //then expand the star and return an array with all the path options 
                char* expanded_path=malloc(sizeof(char)*PATH_MAX);
                memset(expanded_path,0,sizeof(char)*PATH_MAX);

                char ** mypath=parse_path(list_arg[i],&nb_parts,"/");
                int nb_options=0;
                char** options=malloc(sizeof(char*)*PATH_MAX);

                expand_star(mypath,nb_parts,expanded_path,options,&nb_options);

                //concat final array to the argument array
                args_extanded=concat_tab(args_extanded,&size,options,nb_options);
                
                
                //free the allocated memory
                free(options);
                free(mypath);
                free(expanded_path);
              }else{
                
                args_extanded=concat_elem(args_extanded,&size,list_arg[i]);
          
              }
            }//expanding the path

      

      // exec command
      if (strcmp(cmd, "exit") == 0) {
        previous_return_value = my_exit(args_extanded+1, length);
      } else {
        if (strcmp(cmd, "cd") == 0) {
          previous_return_value = my_cd(args_extanded+1, length);
        } else {
          if (strcmp(cmd, "pwd") == 0) {
            previous_return_value = my_pwd(args_extanded+1, length);
          } else {
            //execute external command with the new extanded array of args
            previous_return_value = extern_command(cmd,args_extanded);
            }
          }
          }
          free_struct(args_extanded,size);
          free(list_arg);
        }
      free(line);
      free(trimmed_line);
    } 
 }

int main(int argc, char const *argv[]) {
  rl_outstream = stderr;
  getcwd(current_rep, PATH_MAX);
  treat_signal(true);
  read_cmd();
  return (0);
}
