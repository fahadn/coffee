// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <error.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

/* ERROR CDOES:
 * -1 = RUNNING
 *  0 = EXIT_SUCCESS
 *  1 = EXIT_FAILURE
 *  2 = YET_TO_BE_RUN
 */

void cmd_error(const char* cmd)
{
  if (cmd == NULL)
    exit(1);
  fprintf(stderr, "Command: %s cannot be found.\n", cmd);
  exit(1);
}

bool run_command(command_t c)
{
  if (c == NULL)
  {
    return false;
  }
  // Check IO
  // Set contents of file to be an argument
  if (c->input != NULL)
  {
    size_t i = 0;
    size_t in_size = strlen(c->input);
    // Find end of command list
    while (c->u.word[i] != NULL)
    {
      i++; 
    }
    c->u.word[i] = (char*)malloc(in_size *sizeof(char));
    c->u.word[i] = c->input;
    c->u.word[i+1] = (char*)malloc(sizeof(char));
    c->u.word[i+1] = NULL;
  } 

  // Print stdout to file
  if (c->output != NULL)
  {
    freopen(c->output, "w", stdout);
  }

  // Testing Code
  printf("\n\nCommand: %s \nWith Arguments: %s \n", c->u.word[0], c->u.word[1] );
  
  if (execvp(c->u.word[0], c->u.word) == -1)
  {
    c->status = 1;
    cmd_error(c->u.word[0]);
  }

  // Close output file if set
  if (c->output != NULL)
  {
    fclose(stdout);
  }
  return true;
}

void free_command(command_t c)
{
  switch(c->type)
  {
    case SIMPLE_COMMAND:
      {
        free(c);
        break;
      }
    case SUBSHELL_COMMAND:
      {
        free_command(c->u.subshell_command);
        break;
      }
    case AND_COMMAND:
    case OR_COMMAND:
    case PIPE_COMMAND:
    case SEQUENCE_COMMAND:
      {
        free_command(c->u.command[0]);
        free_command(c->u.command[1]);
        break;
      }
  }
  return;
}

bool recurse_command(command_t c)
{
  int pipefd[2];
  int status = 0;
  pid_t in_pid, out_pid;

  // Error Check
  if (c == NULL)
  {
    return true;
  }

  // Recurse through children
  switch(c->type)
  {
    case AND_COMMAND:
    {
      return recurse_command(c->u.command[0]) && recurse_command(c->u.command[1]);
    }
    case OR_COMMAND:
    {
      return recurse_command(c->u.command[0]) || recurse_command(c->u.command[1]);
    }
    case PIPE_COMMAND:
    {
      status = 0;
      pipe(pipefd);
      // Generate output to pipe
      out_pid = fork();
      if (out_pid == 0)
      {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        recurse_command(c->u.command[0]);
      }
      else if (out_pid > 0)
        wait(&status);
      else if (out_pid < 0)
      {
        fprintf(stderr, "Fork Error");
        exit(1);
      }

      // Generate input to pipe
      in_pid = fork();
      if (in_pid == 0)
      {
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[1]);
        recurse_command(c->u.command[1]);
      }
      else if (in_pid > 0)
        wait(&status);
      else if (in_pid < 0)
      {
        fprintf(stderr, "Fork Error");
        exit(1);
      }
      return true;

    }
    case SEQUENCE_COMMAND:
    {
      recurse_command(c->u.command[0]);
      recurse_command(c->u.command[1]);
      return true;
    }
    case SUBSHELL_COMMAND:
    {
      return recurse_command(c->u.subshell_command);
    }
    case SIMPLE_COMMAND:
    {
      status = 0;
      // Put command in separate process
      pid_t pid = fork();
      if (pid > 0)
        wait(&status);
      else if (pid == 0)
      {
        if (!isspace(c->u.word[0][0]))
        {
          run_command(c);
        }
      } 
      else if (pid < 0)
      {
        fprintf(stderr, "Fork Error");
        exit(1);
      }

      if (status > 1)
        return false;
    }
    default:
    {
      return true;
    }
  }
}

int
command_status (command_t c)
{
  return c->status;
}

void
execute_command (command_t c, bool time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  if (time_travel)
  {
    // Implement Me
    error (1, 0, "time travel execution not yet implemented");
    return;
  }
  else
  {
    recurse_command(c);
    free_command(c);
  }


  return;
}
