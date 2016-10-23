#ifndef SIGNALS_H_
#define SIGNALS_H_

#include "essentials.h"

/**
 * the SIGINT handler
 *
 * params: int signum, siginfo_t *sig, void *v as specified in Linux standard document
**/
void int_handler(int signum, siginfo_t *sig, void *v){
  check_sigint = true;
  if (!check_ls_l) {
    check_ls_l = true;
  }
  int i;
  for (i = 0; i < bg_infos_n; ++i){
    fprintf(stdout, "%s\n", bg_infos[i]); // print background infos
  }
  if (bg_infos_n) free(bg_infos);
  bg_infos_n = 0;
  if (foreground_pid != -1) check_print_again = true;
}

/**
 * the SIGCHLD handler
 *
 * params: int signum, siginfo_t *sig, void *v as specified in Linux standard document
**/
void chld_handler(int signum, siginfo_t *sig, void *v){

  // timeX case, only in foreground mode
  if (check_timeX){
    // start reading
    char* namebuf = (char*)calloc(MAX_FILENAME_LENGTH, sizeof(char));
    if(namebuf){
      sprintf(namebuf, "/proc/%d/stat", sig->si_pid);
      FILE* f = fopen(namebuf, "r");
      if(f){
        size_t size = fread(namebuf, sizeof(char), MAX_FILENAME_LENGTH, f);
        if(size > 0 &&'\n'== namebuf[size-1]){
          namebuf[size-1]='\0';
        }
        fclose(f);
      }
    }

    int buff_i, cmd_i;
    strtok(namebuf, " ");
    char* proc_name = strtok(NULL, " ");
    for (cmd_i = 0; proc_name[cmd_i+1] != '\0'; ++cmd_i){
      proc_name[cmd_i] = proc_name[cmd_i+1];
    }
    proc_name[cmd_i-1] = '\0';

    for (buff_i = 3; buff_i < 14; ++buff_i){
      strtok(NULL, " ");
    }
    double u_time = atol(strtok(NULL, " "))*1.0f/sysconf(_SC_CLK_TCK); // UTIME
    double s_time = atol(strtok(NULL, " "))*1.0f/sysconf(_SC_CLK_TCK); // STIME
    for (buff_i = 16; buff_i < 22; ++buff_i){
      strtok(NULL, " ");
    }

    double starttime = atol(strtok(NULL, " "))*1.0f/sysconf(_SC_CLK_TCK);
    char* systimebuff = (char*)calloc(MAX_FILENAME_LENGTH, sizeof(char));
    if(systimebuff){
      sprintf(systimebuff, "/proc/uptime");
      FILE* f = fopen(systimebuff, "r");
      if(f){
        size_t size = fread(systimebuff, sizeof(char), MAX_FILENAME_LENGTH, f);
        if(size > 0 &&'\n'== systimebuff[size-1]){
          systimebuff[size-1]='\0';
        }
        fclose(f);
      }
    }
    double endtime = atof(systimebuff);
    double r_time = endtime - starttime; // RTIME

    //output timeX info
    fprintf(stdout, "\n");
    fprintf(stdout, "PID\tCMD\t\tRTIME\t\tUTIME\t\tSTIME\t\t\n");
    fprintf(stdout, "%d\t%s\t\t%.2lf s\t\t%.2lf s\t\t%.2lf s\t\t\n", sig->si_pid, proc_name, r_time, u_time, s_time);
    return;
  }

  // handle the case when SIGINT generates in foreground child process and therefore generates SIGCHLD
  // in this case, we should not print anything out
  if(check_sigint){
    check_sigint = false;
    check_no_new = true;
    return;
  }

  // foreground
  if (sig->si_pid == foreground_pid) {
    check_no_new = false;
    return;
  }

  //pipe cases
  int pipe_i;
  for (pipe_i = 0; pipe_i < 5; ++pipe_i){
    if (pipe_pids[pipe_i] == sig->si_pid){
      check_no_new = true;
      return;
    }
  }

  // set no new "## myshell " to 1
  check_no_new = true;
  

  // normal cases: SIGCHLD generates when background process finishes
  char* namebuf = (char*)calloc(MAX_FILENAME_LENGTH, sizeof(char));
  if(namebuf){
    sprintf(namebuf, "/proc/%d/comm", sig->si_pid);
    FILE* f = fopen(namebuf, "r");
    if(f){
      size_t size = fread(namebuf, sizeof(char), MAX_FILENAME_LENGTH, f);
      if(size > 0 &&'\n'== namebuf[size-1]){
        namebuf[size-1]='\0';
      }
      fclose(f);
    }
  }

  // handle all outputs
  if (foreground_pid == -1) {
    fprintf(stdout, "[%d] %s Done\n", sig->si_pid, namebuf);
  } else {
    if (!bg_infos){
      bg_infos = malloc(bg_infos_n_max * sizeof(char*));
      // check if fail to allocate memory
      if (!bg_infos){
        fprintf(stderr, "myshell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    } else if (bg_infos_n == bg_infos_n_max - 1) {
      bg_infos_n_max*=2;
      char ** bg_infos_backup = bg_infos;
      bg_infos = realloc(bg_infos, bg_infos_n_max * sizeof(char*));
      // check if fail to reallocate memory
      if (!bg_infos) {
        free(bg_infos_backup);
        fprintf(stderr, "myshell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
    bg_infos[bg_infos_n] = (char*)calloc((MAX_FILENAME_LENGTH+20), sizeof(char));
    sprintf(bg_infos[bg_infos_n++], "[%d] %s Done", sig->si_pid, namebuf);
  }
}

/**
 * the SIGUSR1 handler
 *
 * params: int signum, siginfo_t *sig, void *v as specified in Linux standard document
**/
void usr1_handler(int signum, siginfo_t *sig, void *v){
  // for later use in child
  check_usr1 = true;
}

/**
 * init all the signal handlers
**/
void init_signals(){

  // handle SIGCHLD
  struct sigaction sa_chld_bg;
  sigaction(SIGCHLD, NULL, &sa_chld_bg);
  sa_chld_bg.sa_flags = SA_SIGINFO;
  sa_chld_bg.sa_sigaction = chld_handler;
  sigaction(SIGCHLD, &sa_chld_bg, NULL);
  
  // handle SIGINT
  struct sigaction sa_int;
  sigaction(SIGINT, NULL, &sa_int);
  sa_int.sa_flags = SA_SIGINFO;
  sa_int.sa_sigaction = int_handler;
  sigaction(SIGINT, &sa_int, NULL);

  // handle SIGUSR1
  struct sigaction sa_usr1;
  sigaction(SIGUSR1, NULL, &sa_usr1);
  sa_usr1.sa_flags = SA_SIGINFO;
  sa_usr1.sa_sigaction = usr1_handler;
  sigaction(SIGUSR1, &sa_usr1, NULL);
}

#endif