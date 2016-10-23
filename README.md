# myshell

Implementation of bash shell in C, for the [Operating System Course](http://www.cs.hku.hk/programme/course_info.jsp?infile=2013/comp3230.html "HKU COMP3230 Principles of Operating Systems") in HKU.

<img src="https://github.com/irsisyphus/pictures/raw/master/myshell/myshell.png" width = "642" height = "396" alt="Host and Port" align=center />

### Development Platform
 - 64-bit ubuntu 14.04 LTS
 - gcc (Ubuntu 4.8.4-2ubuntu1~14.04.3) 4.8.4

### Usage
 - Compilation: `gcc src/myshell.c -o myshell -Wall`
 - Run: `./myshell`

### Enabled Functionalities
 * Commands in `/bin:/usr/bin`
 * pipe
 * background mode
 * `timeX`: a built-in function for counting execution time, support pipe and foreground commands.
  * For instance, `timeX ls -l | wc`
 * `exit`

### Blocked Functionalities
 - SIGINT signal in `myshell`. You cannot exit this program using SIGINT signal. Use `exit` instead.

### Disabled or Unimplemented Functionalities
 - `cd`: changing folder is not supported in `myshell`
