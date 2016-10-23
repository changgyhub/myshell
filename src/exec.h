#ifndef EXEC_H_
#define EXEC_H_

#include "essentials.h"

/**
 * check if the user requires to exit the program
 *
 * return: EXEC_NORMAL_EXIT if the user does not require to exit
           EXEC_SPECIAL_EXIT if "exit" command is incorrectly input
           EXEC_FORCE_QUIT if the user correctly requires to exit
**/
int exit_check(char **args){
  // check if command is "exit"
  if (!strcmp(args[0], "exit")){
    if (args[1]){
      fprintf(stderr,"myshell: \"exit\" with other arguments!!!\n");
      return EXEC_SPECIAL_EXIT;
    }
    return EXEC_FORCE_QUIT;
  } else if (!strcmp(args[0], "exit&")){
    fprintf(stderr,"myshell: \"exit\" with other arguments!!!\n");
    return EXEC_SPECIAL_EXIT;
  }
  return EXEC_NORMAL_EXIT;
}

/**
 * the whole execution process
 *
 * param: char ***pipes: the arguments to be executed, in the form of
                         char* argument -> char** command -> char ***pipes
 * return: EXEC_NORMAL_EXIT if the user does not require to exit
           EXEC_SPECIAL_EXIT if "exit" command is incorrectly input
           EXEC_FORCE_QUIT if the user correctly requires to exit
           some flags with prefix "check_" if necessary
**/
int exec(char ***pipes){
  check_ls_l = false;
  int status;
  int background_flag = 0; // background mode = 1, otherwise 0
  // check if a signal is sent, typically SIGINT
  if (check_sigint) {
    check_sigint = false;
    fprintf(stdout, "\n");
    return EXEC_NORMAL_EXIT;
  }

  // check if there are no input
  if (!pipes) return EXEC_NORMAL_EXIT;

  int pfd[MAX_PIPES][2];
  char ** args;
  int p;
  // open pipes
  for(p = 0; p < pipe_num; ++p){
    pipe(pfd[p]);
  }

  // start checking special conditions
  for(p = 0; p <= pipe_num; ++p) {
    args = pipes[p];

    // check if there are no input
    if (!args[0]) return EXEC_NORMAL_EXIT;

    // check if the command is "exit"
    // only when no pipe
    if (!p){
      int check_exit = exit_check(args);
      if (check_exit != EXEC_NORMAL_EXIT){
      	if (pipe_num) {
      		fprintf(stderr, "myshell: \"exit\" with other arguments!!!\n");
      		return EXEC_SPECIAL_EXIT;
      	} else return check_exit;
      }
    }

    // check if command shoule be run in background mode
    int i;
    for (i = 0; args[i]; ++i){
      if (!strcmp(args[i], "&")){
        if (p != pipe_num || args[i+1]){
          fprintf(stderr,"myshell: \'&\' should not appear in the middle of the command line\n");
          return EXEC_SPECIAL_EXIT;
        } else if (!i){
          fprintf(stderr,"myshell: unexpected token \'&\'\n");
          return EXEC_SPECIAL_EXIT;
        } else {
          background_flag = 1;
          args[i] = NULL;
        }
      }
    }
    if (!background_flag){
      char *findc = strchr(args[i-1], '&');
      if (findc){
        if (!strcmp(args[i-1], "&&")){
          fprintf(stderr,"myshell: \'&\' should not appear in the middle of the command line\n");
          return EXEC_SPECIAL_EXIT;
        } else {
          *findc = '\0';
          background_flag = 1;
        }
      }
    }
  }
  
  // start forking after finish checking special conditions
  for(p = 0; p <= pipe_num; ++p) {
    args = pipes[p];
    // check if timeX
    if (!p){
    	if (!strcmp(args[0], "timeX")){
  	    if (!args[1] && !pipe_num){
  	      fprintf(stderr,"myshell: \"timeX\" cannot be a standalone command\n");
  	      return EXEC_SPECIAL_EXIT;
  	    } else if (background_flag){
  	      fprintf(stderr,"myshell: \"timeX\" cannot be run in background mode\n");
  	      return EXEC_SPECIAL_EXIT;
  	    } else {
  	      int arg_i;
  	      for (arg_i = 0; args[arg_i+1]; ++arg_i){
  	      	args[arg_i] = args[arg_i+1];
  	      }
  	      args[arg_i] = NULL;
  	      check_timeX = true; // since timeX cannot be runned in background mode, we use it as a global variable
  	    }
  	  }
    }

    check_usr1 = false;
    pid_t pid = fork();

    // failure
    if (pid < 0) {
      fprintf(stderr,"myshell: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    } 

    // child process
    else if (pid == 0) {
      // wait SIGUSR1 handler
      while (!check_usr1){};
      check_usr1 = false;
      // change pipe
      // note that this can only be done after forking !!!
      if (pipe_num){
        int pipe_i;
        if (!p){
          close(pfd[0][0]); // close pipe0 read end
          dup2(pfd[0][1], 1); //set pipe0 write end to stdout
          //close other pipes
          for (pipe_i = 1; pipe_i < pipe_num; ++pipe_i){
            close(pfd[pipe_i][0]);
            close(pfd[pipe_i][1]);
          }
        } else if (p == pipe_num){
          close(pfd[pipe_num-1][1]); //close pipe(pipe_num-1) write end
          dup2(pfd[pipe_num-1][0], 0); //set pipe(pipe_num-1) read end to stdin
          //close other pipes
          for (pipe_i = 0; pipe_i < pipe_num-1; ++pipe_i){
            close(pfd[pipe_i][0]);
            close(pfd[pipe_i][1]);
          }
        } else {
          for (pipe_i = 0; pipe_i < pipe_num; ++pipe_i){
            if (pipe_i == p) {
              dup2(pfd[pipe_i][1], 1); //set write end to stdout
            } else {
              close(pfd[pipe_i][1]);
            }
            if (pipe_i == p-1) {
              dup2(pfd[pipe_i][0], 0); //set read end to stdin
            } else {
              close(pfd[pipe_i][0]);
            }
          }
        }
      }

      if (background_flag){
        // we need to set child a different process group so that 
        // it will not receive the signals generated by the controlling terminal
        if (setpgid(0, 0)){
          fprintf(stderr,"myshell: %s\n", strerror(errno));
          exit(EXIT_FAILURE);
        }
      }

      // execute command
      if (execvp(args[0], args) == -1) {
        // unsuccessful run of command
        if (pipe_num){
          fprintf(stderr, "myshell: Fail to execute \'%s\': %s\n", args[0], strerror(errno));
          for (p = 0; p < pipe_num; ++p){
            close(pfd[p][0]);
            close(pfd[p][1]);
          }
        } else {
          fprintf(stderr, "myshell: \'%s\': %s\n", args[0], strerror(errno));
        }
        exit(EXIT_FAILURE);
      }
    }

    // parent process
    else {
      // send SIGUSR1 signal
      kill(pid, SIGUSR1);
      // start waiting child process
      if (background_flag) {
        if (!pipe_num){
          waitpid(pid, &status, WNOHANG);
        } else if (p == pipe_num){
          for (p = 0; p < pipe_num; ++p){
            close(pfd[p][0]);
            close(pfd[p][1]);
          }
          waitpid(pid, &status, WNOHANG);
        }
      } else {
        foreground_pid = pid;
        if (!p){
          memset(pipe_pids, -2, sizeof(pipe_pids));
        }
        pipe_pids[p] = pid;
        if (p == pipe_num){
          // close all pipes, must be called before waitpid()
          for (p = 0; p < pipe_num; ++p){
            close(pfd[p][0]);
            close(pfd[p][1]);
          }
          // timeX process
          int pp_i;
          if (check_timeX) {
            for (pp_i = 0; pp_i <= pipe_num; ++pp_i){
              while(waitid(P_PID, pipe_pids[pp_i], NULL, WNOWAIT|WEXITED)==-1){}
            }
            check_sigint = false;
          } else {
            for (pp_i = 0; pp_i <= pipe_num; ++pp_i){
              while(waitpid(pipe_pids[pp_i], &status, 0) == -1){}
            }
          }
          if (pipe_num && !status && !check_timeX) {
            fprintf(stdout, "## myshell $ ");
            check_ls_l = true;
          }
          if (WTERMSIG(status) == 15){ // killed by SIGKILL
            check_killed = true;
          }
        }
      }
    }
  }
  return EXEC_NORMAL_EXIT;
}

#endif