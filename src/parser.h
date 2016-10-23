#ifndef PARSER_H_
#define PARSER_H_

#include "essentials.h"

/**
 * read a whole line of characters
 *
 * return: char * consisting of the characters
**/
char *read_line(){
  int bufsize = LINE_BUFSIZE, pos = 0;
  char c, *line = malloc(sizeof(char) * bufsize);
  
  // check if fail to allocate memory
  if (!line) {
    fprintf(stderr, "myshell: allocation error\n");
    exit(EXIT_FAILURE);
  }
  
  // start reading the whole line
  while ((c = getchar())) {
    if (c == EOF || c == '\n') {
      line[pos] = '\0';
      return line;
    } else {
      line[pos++] = c;
    }
    // double the bufsize if not enough space
    if (pos >= bufsize) {
      bufsize *= 2;
      line = realloc(line, bufsize);
      // check if fail to reallocate memory
      if (!line) {
        fprintf(stderr, "myshell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
  return line;
}

/**
 * split the line of characters into the form of
 * char *argument -> char **command -> char ***pipes
 *
 * return: char *** consiting of the pipes
**/
char ***split_line(char *line){
  int bufsize = ARG_BUFSIZE, pipesize = MAX_PIPES + 1, pos = 0;
  char *arg, **args = malloc(bufsize * sizeof(char*)), **args_backup;
  char ***pipes = malloc(pipesize  * sizeof(char**));
  
  // check if fail to allocate memory
  if (!args || !pipes) {
    fprintf(stderr, "myshell: allocation error\n");
    exit(EXIT_FAILURE);
  }
  
  // start spliting
  arg = strtok(line, ARG_SPLIT);
  while (arg) {
    // detect pipe
    if (!strcmp(arg, "|")){
      if (pipe_num == MAX_PIPES){
        fprintf(stderr, "myshell: The system does not support more than %d pipes\n", MAX_PIPES);
        return NULL;
      }
      args[pos] = NULL;
      pipes[pipe_num++] = args;
      pos = 0;
      args = malloc(bufsize * sizeof(char*));
    } else {
      args[pos++] = arg;
      // double the bufsize if not enough space
      if (pos >= bufsize) {
        bufsize *= 2;
        args_backup = args;
        args = realloc(args, bufsize * sizeof(char*));
        // check if fail to reallocate memory
        if (!args) {
          free(args_backup);
          fprintf(stderr, "myshell: allocation error\n");
          exit(EXIT_FAILURE);
        }
      }
    }
    arg = strtok(NULL, ARG_SPLIT);
  }
  if (!pipe_num){
    args[pos] = NULL;
    pipes[0] = args;
  } else {
    if (!pos){
      fprintf(stderr, "myshell: Incomplete '|' sequence\n");
      free(pipes);
      return NULL;
    } else {
      args[pos] = NULL;
      pipes[pipe_num] = args;
    }
  }
  return pipes;
}

#endif