#ifndef MYSHELL_H_
#define MYSHELL_H_

#include "essentials.h"
#include "signals.h"
#include "parser.h"
#include "exec.h"

/**
 * check the loop status, handle special cases and signals before
 * each reading loops starts.
**/
void loop_check(){
	int i, status;
  check_timeX = false;

  // handle background info
  while(waitpid(-1, &status, WNOHANG)>0){} //collect <defunct> child

  for (i = 0; i < bg_infos_n; ++i){
      fprintf(stdout, "%s\n", bg_infos[i]); // print background infos
  }
  if (bg_infos_n) free(bg_infos);
  bg_infos_n = 0;
  if (!check_no_new || check_killed) {
    if (!check_ls_l) {
      fprintf(stdout, "## myshell $ ");
      check_print_again = false;
    }
  }
  if (check_print_again) fprintf(stdout, "## myshell $ ");
  foreground_pid = -1; // reset to foreground mode
  pipe_num = 0;
  check_no_new = false;
  check_killed = false;
  check_print_again = false;
}

/**
 * The reading loop, contains the whole process
 * from parsing to executing one line command
 * 
 * return: EXIT_SUCCESS if the user requried to exit the program
 *         otherwise the loop continues without ending
**/
int read_loop(){
  char *line, ***pipes;
  int status;
  init_signals();
  do {
  	loop_check();
    line = read_line();
    pipes = split_line(line);
    status = exec(pipes);
    free(line);
    free(pipes);
  } while (status != EXEC_FORCE_QUIT);
  fprintf(stdout, "myshell: Terminated\n");
  return EXIT_SUCCESS;
}


#endif