/* 114110213 tstone97 0305 */
#include "safe-fork.h"
#include "split.h"
#include <string.h>
#include <err.h>
#include <sysexits.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

void free_string_list (char **strings);

/* breaks apart command line arguments into the target program and target 
arguments and runs the target program through exec in two different modes - it
prints out the standard input either line by line or in standard mode which
just prints the whole standard input as is */
int main(int argc, char *argv[]) {
  pid_t pid;
  int i;
  char *target_program = NULL;
  int pos = 0;
  char **target_args = malloc(sizeof(char *));
  int index = 0;
  char **split_lines;
  char input_line[1000];
  int i_mode = 0;

    
  *target_args = NULL;
  
#if 1
  /* has more than one argument */
  if (argc > 1) {
    
    /* if the program should be run in standard mode */
    if (strcmp("-i", argv[1]) != 0) {
      target_program = malloc(strlen(argv[1]) + 1);
      
      /* copy the part of the command line that refers to the target program 
	 into target_program */
      strcpy(target_program, argv[1]);
    }

    /* if the program should be run in line by line mode */
    else {
      
      /* the echo case where -i is the only argument */
      if (argc == 2) {
	target_program = malloc(5);
	strcpy(target_program, "echo");
      }

      /* the line by line mode where there are more than 2 arguments */
      else {
	target_program = malloc(strlen(argv[2]) + 1);
	strcpy(target_program, argv[2]);
      }
      /* field that keeps track of which mode the program should be run in */
      i_mode = 1;
    }

    /* allocates target_args for the target program */
    target_args[pos] = malloc(strlen(target_program) + 1);
    strcpy(target_args[pos], target_program);
    pos++;
    target_args = realloc(target_args, (pos + 1) * sizeof(char *));
    /* sets the last string to NULL */
    target_args[pos] = NULL;

    /* if the target program has arguments */
    for (i = 2; i <= (argc - 1); i++) {
      target_args[pos] = malloc(strlen(argv[i]) + 1);
      /* copies over the target arguments into the target_args double pointer
	 to an array of strings */
      strcpy(target_args[pos], argv[i]);
      pos++;
      /* reallocates memory in target_args to fit the new addition and sets
	 the last string to NULL */
      target_args = realloc(target_args, (pos + 1) * sizeof(char *));
      target_args[pos] = NULL;
    }
  }

  /* if only have one argument in the command line */
  else {
    target_program = malloc(5);
    strcpy(target_program, "echo");
    target_args[pos] = malloc(strlen(target_program) + 1);
    strcpy(target_args[pos], target_program);
    pos++;
    target_args = realloc(target_args, (pos + 1) * sizeof(char *));
    target_args[pos] = NULL;

  }

  /* reads in the standard input from a file and stores that in a string and 
     adds this string to the target_args array to be used in exec */
  while (!feof(stdin)) {
    
    /* standard mode */
    if (!i_mode) {

      /* while fgets produces a non NULL return value split the line that was
	 read in from stdin */
      while (fgets(input_line, 1000, stdin)) {
	split_lines = split(input_line);
	index = 0;

	/* adds the split lines to the target_args array */
	while (split_lines[index] != NULL) {
	  target_args[pos] = malloc(strlen(split_lines[index]) + 1);
	  strcpy(target_args[pos], split_lines[index]);
	  pos++;
	  /* reallocates memory for the next string to be added to 
	     target_args */
	  target_args = realloc(target_args, (pos + 1) * sizeof(char *));
	  target_args[pos] = NULL;	  
	  index++;
	}

	/* calls helper function to free all the strings in split_lines */
	free_string_list(split_lines);
	split_lines = NULL;
	
      }

      pid = safe_fork();

      /* child process - executes the target program with the 
	 target arguments */ 
      if (pid == 0) {
	execvp(target_program, target_args);
      }

      /* parent process - waits for the child to execute the program and gets
	 the status that the child exited with */
      else if (pid > 0) {
	int status;
	wait(&status);
	if (status != 0)
	  exit(1);
      }
    }

    /* line by line mode */
    else {

      /* while fgets produces a non NULL return value split the line that was
	 read in from stdin */
      while(fgets(input_line, 1000, stdin)) {
	split_lines = split(input_line);

	/* adds the split lines to the target_args array */
	while (split_lines[index] != NULL) {
	  target_args[pos] = malloc(strlen(split_lines[index]) + 1);
	  strcpy(target_args[pos], split_lines[index]);
	  pos++;
	  /* reallocates memory for the next string to be added to 
	     target_args */
	  target_args = realloc(target_args, (pos + 1) * sizeof(char *));
	  target_args[pos] = NULL;	  
	  index++;
	}
	
	pid = safe_fork();

	/* child process - executes the target program with the 
	   target arguments */  
	if (pid == 0) {
	  execvp(target_program, target_args);
	}

	/* parent process - waits for the child to execute the program and gets
	   the status that the child exited with */
	else if (pid > 0) {
	  int status;
	  wait(&status);
	  if (status != 0)
	    exit(1);
	}

	/* free strings lists - split_lines and target_args */
	free_string_list(target_args);
	free_string_list(split_lines);
	split_lines = NULL;

	/* after freeing the lists need to create them again for the next line
	   in line by line mode */
	pos = 0;
	target_args = malloc(sizeof(char *));
	target_args[pos] = malloc(strlen(target_program + 1));
	strcpy(target_args[pos], target_program);
	pos++;
	target_args = realloc(target_args, (pos + 1) * sizeof(char *));
	target_args[pos] = NULL;

	/* if the target program has arguments */
	for (i = 2; i <= (argc - 1); i++) {
	  target_args[pos] = malloc(strlen(argv[i]) + 1);
	  strcpy(target_args[pos], argv[i]);
	  pos++;
	  target_args = realloc(target_args, (pos + 1) * sizeof(char *));
	  target_args[pos] = NULL;
	}
	
	
      }
    }

    /* frees all the rest of the lists */
    free_string_list(target_args);
    free_string_list(split_lines);
    free(target_program);
    target_args = NULL;
    split_lines = NULL;
    target_program = NULL;

    
  }


  #endif
  
  return 0;
}

  /* frees all of the strings in an array of strings and then frees the pointer
   and sets it to NULL */
void free_string_list(char **strings){
  char **temp = strings;

  if (strings != NULL) {
    /* while the pointer to the strings isnt NULL frees all the string pointers */
    while (*strings != NULL) {
      free(*strings);
      *strings = NULL;
      strings++;
    }
  
    free(temp);
  }
  temp = NULL;
}
