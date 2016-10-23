#ifndef ESSENTIALS_H_
#define ESSENTIALS_H_

// ************************************
//              libraries
// ************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

// ************************************
//               macros
// ************************************
// Maximum filename length
#define MAX_FILENAME_LENGTH 256
// Default buffer size of a line
#define LINE_BUFSIZE 512
// Default buffer size of an argument
#define ARG_BUFSIZE 32
// Maximum number of pipes
#define MAX_PIPES 4
// Set of spliting characters
#define ARG_SPLIT " \a\n\r\t"
// Denote normal exit events of one execution
#define EXEC_NORMAL_EXIT -1
// Denote special exit events of one execution
#define EXEC_SPECIAL_EXIT -2
// Denote force quit events by one execution
#define EXEC_FORCE_QUIT -3
// Define bool
#define bool unsigned int
// Define true
#define true 1
// Define false
#define false 0

// ************************************
//          global variables
// ************************************
bool check_sigint = false; // check if a SIGINT is sent
bool check_no_new = false; // check if no "## myshell " to be printed
bool check_ls_l = false; //whether a "## myshell " has been printed by "ls -l"
bool check_killed = false; // whether killed bu SIGKILL
bool check_timeX = false; // whether "timeX" is enabled
bool check_print_again = false; //print again "## myshell " in SIGINT
bool check_usr1 = false; // check if a SIGUSR1 is sent

pid_t foreground_pid = -1; // store foreground pid

char ** bg_infos; // store background SIGCHLD infos
unsigned int bg_infos_n = 0; // store num of bg_infos
unsigned int bg_infos_n_max = 10; // store max num of bg_infos

int pipe_num = 0; // number of pipes
int pipe_pids[MAX_PIPES+1]; // store pids of pipe

// ************************************
//        function declarations
// ************************************

/**
 * check the loop status, handle special cases and signals before
 * each reading loops starts.
**/
void loop_check();

/**
 * The reading loop, contains the whole process
 * from parsing to executing one line command
 * 
 * return: EXIT_SUCCESS if the user requried to exit the program
 *         otherwise the loop continues without ending
**/
int read_loop();

/**
 * check if the user requires to exit the program
 *
 * return: EXEC_NORMAL_EXIT if the user does not require to exit
           EXEC_SPECIAL_EXIT if "exit" command is incorrectly input
           EXEC_FORCE_QUIT if the user correctly requires to exit
**/
int exit_check(char **args);

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
int exec(char ***pipes);

/**
 * read a whole line of characters
 *
 * return: char * consisting of the characters
**/
char *read_line();

/**
 * split the line of characters into the form of
 * char *argument -> char **command -> char ***pipes
 *
 * return: char *** consiting of the pipes
**/
char ***split_line(char *line);

/**
 * the SIGINT handler
 *
 * params: int signum, siginfo_t *sig, void *v as specified in Linux standard document
**/
void int_handler(int signum, siginfo_t *sig, void *v);


/**
 * the SIGCHLD handler
 *
 * params: int signum, siginfo_t *sig, void *v as specified in Linux standard document
**/
void chld_handler(int signum, siginfo_t *sig, void *v);


/**
 * the SIGUSR1 handler
 *
 * params: int signum, siginfo_t *sig, void *v as specified in Linux standard document
**/
void usr1_handler(int signum, siginfo_t *sig, void *v);

/**
 * init all the signal handlers
**/
void init_signals();

#endif