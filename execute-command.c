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
#include <time.h>

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
  //printf("\n\nCommand: %s \nWith Arguments: %s \n", c->u.word[0], c->u.word[1] );

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
        printf("NO ERROR HERE\n");
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
  return true;
}

bool break_tree(command_t c, command_t **output_cmd_array, int* array_size)
{
  if ( c == NULL)
    return false;
  switch(c->type)
  {
    // In order traversal to break tree.
    // Makes leaf nodes NULL
    case AND_COMMAND:
    case OR_COMMAND:
    case PIPE_COMMAND:
    case SEQUENCE_COMMAND:
      {
        break_tree(c->u.command[0], output_cmd_array, array_size);
        (*output_cmd_array) = (command_t*)realloc((*output_cmd_array), ((*array_size)+1)*sizeof(command_t)*100);
        (*output_cmd_array)[(*array_size)] = c;
        (*array_size)++;
        break_tree(c->u.command[1], output_cmd_array, array_size);
        break;
      }
      // Adds command to command list
      // Subshell does not get broken up
    case SUBSHELL_COMMAND:
    case SIMPLE_COMMAND:
      {
        (*output_cmd_array) = (command_t*)realloc((*output_cmd_array), ((*array_size)+1)*sizeof(command_t)*100);
        (*output_cmd_array)[(*array_size)] = c;

        (*array_size)++;
        break;
      }
    default:
      {
        return false;
      }
  }
  return true;

}

bool form_tree(command_t *c, command_t output_cmd)
{
  c=c;
  output_cmd = output_cmd;
  return true;
}

  int
command_status (command_t c)
{
  return c->status;
}

void test_output_cmd(command_t *cmd_array, int array_size)
{ 
  // Test break tree
  int i = 0;
  for (i = 0; i < array_size ;i++)
  {
    switch(cmd_array[i]->type)
    {
      case AND_COMMAND:
        printf("Type: AND_COMMAND\n"); break;
      case SEQUENCE_COMMAND:
        printf("Type: SEQENCE_COMMAND\n"); break;
      case OR_COMMAND:
        printf("Type: PIPE_COMMAND\n"); break;
      case PIPE_COMMAND:
        printf("Type: PIPE_COMMAND\n"); break;
      case SIMPLE_COMMAND:
        printf("Type: SIMPLE_COMMAND\n Word: %s\n", cmd_array[i]->u.word[0]); break;
      case SUBSHELL_COMMAND:
        printf("Type: SUBSHELL_COMMAND\n"); break;
    }
  }
}

/***********************
 * GLOBAL VARIABLES  *
 ***********************/
char ***depend_read_list = NULL;
int* depend_read_count = NULL;

char ***depend_write_list = NULL;
int* depend_write_count = NULL;
/***********************
 * ********************/

// Depend list has NULL at end of list 
bool make_depend_list(command_t c, char*** depend_read_list,  int* read_count, 
    char*** depend_write_list, int* write_count )
{
  if ( c == NULL )
    return false;
  if (depend_read_count == NULL)
  {
    depend_read_count = malloc(sizeof(int));
    depend_read_count = 0;
  }
  if (depend_write_count == NULL)
  {
    depend_write_count = malloc(sizeof(int));
    depend_write_count = 0;
  }
  switch(c->type)
  {
    case AND_COMMAND:
    case OR_COMMAND:
    case PIPE_COMMAND:
    case SEQUENCE_COMMAND:
      {
        return make_depend_list(c->u.command[0], depend_read_list, read_count, depend_write_list, write_count) &&
          make_depend_list(c->u.command[1], depend_read_list, read_count, depend_write_list, write_count);
      }
    case SUBSHELL_COMMAND:
      {
        return make_depend_list(c->u.subshell_command, depend_read_list, read_count, depend_write_list, write_count);
      }
    case SIMPLE_COMMAND:
      {
        if (c->input != NULL)
        {
          (*depend_read_list) = (char**) realloc((*depend_read_list), (*read_count+10)*sizeof(char*));
          (*depend_read_list)[*read_count] = strdup(c->input);
          (*read_count)++;
        }
        if (c->output != NULL)
        {
          (*depend_write_list) = (char**) realloc((*depend_write_list), (*write_count+10)*sizeof(char*));
          (*depend_write_list)[*write_count] = strdup(c->output);
          (*write_count)++;
        }
        return true;
      }
    default:
      {
        return true;
      }
  }
}

// Array of pointers pointing to arrays of c-strings
bool is_list_empty(char*** list)
{
  return list == NULL;
}

// Adds dependency to global list.  First array element at 0 set to pid
bool add_to_list(char**** list, int *list_size, pid_t p, char** depend, int depend_size)
{
  if (depend == NULL || depend_size < 0)
    return false;
  if (*list == NULL)
    (*list) = malloc(sizeof(char**)*100);

  (*list) = realloc((*list), ((*list_size)+10)*sizeof(char**));
  (*list)[*list_size] = (char**) realloc((*list)[*list_size], (*list_size+100)*sizeof(char*));
  char pid[100];
  sprintf(pid,"%d",p);
  (*list)[*list_size][0] = strdup( pid );
  int i = 1;
  for (i = 1; i < depend_size+1; i++)
  {
    (*list)[*list_size][i] = strdup(depend[i-1]);
  }
  (*list_size)++;
  return true;

}

// Returns pid of dependency when any command in depend is in list
// returns negative pid when not found
pid_t find_match(char *** list, int list_size, char** depend, int depend_size, int id)
{
  if (list == NULL)
    return -1;
  int i = 0;
  int j = 0;
  int k = 0;
  char pid[100];
  sprintf(pid,"%d",id);
  for (i = 0; i < list_size; i++)
  {
    if (list[i] == NULL)
      return -1;
    for (j = 0; j < depend_size; j++)
    {
      k = 0;
      while (list[i][k] != NULL)
      {
        /*
           printf("depend[j]: %s\n",depend[j]);
           printf("list[i][k]: %s\n",list[i][k]);
           printf("pid: %s\n",pid);
           printf("list[i][0]: %s\n",list[i][0]);
           */
        if (strcmp(depend[j], list[i][k]) == 0 &&
            strcmp(pid, list[i][0]) != 0)
        {
          return atoi(list[i][0]);
        }
        k++;
      }
    }
  }
  //printf("returned -1\n");
  return -1;
}

// Remove pid and its dependency from list
bool remove_from_list(char ****list, size_t list_size, pid_t p)
{
  size_t i = 0;
  size_t j = 0;
  char pid[100];
  sprintf(pid,"%d",p);
  for (i = 0; i < list_size; i++)
  {
    if ((*list)[i] != NULL)
    {
      if (strcmp((*list)[i][0], pid) == 0)
      {
        while ((*list)[i][j] != NULL)
        {
          (*list)[i][j] = NULL;
          j++;
        }
        return true;
      }
    }
  }
  return false;
}

void print_list(char** list, int list_size)
{
  if (list == NULL || list_size <0)
  {
    printf("List Array is NULL\n");
    return;
  }
  int i = 0;

  printf("List Size: %d\n", list_size);
  for (i =0; i < list_size;i++)
  {
    printf("Element %d: %s\n", i, list[i]);
  }
  return;
}

  void
execute_command (command_t c, bool time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  if (time_travel)
  {
    pid_t match;
    int status = 0;
    char** local_read_list = NULL;
    // Initialize Local Dependencies
    int* read_count = (int*)malloc(sizeof(int));
    *read_count = 0;

    char **local_write_list = NULL;
    int* write_count = (int*)malloc(sizeof(int));
    *write_count = 0;

    // Initialize Globals
    if(depend_read_count == NULL)
    {
      depend_read_count = malloc(sizeof(int));
      *depend_read_count = 0;
    }
    if(depend_write_count == NULL)
    {
      depend_write_count = malloc(sizeof(int));
      *depend_write_count = 0;
    }

    // Make Dependency List
    make_depend_list(c, &local_read_list,  read_count, 
        &local_write_list, write_count);

    // Add Dependencies
    int r = rand() % 100; // Unique ID for every command
    add_to_list(&depend_read_list, depend_read_count, r, local_read_list, *read_count);
    add_to_list(&depend_write_list, depend_write_count, r, local_write_list, *write_count);

    // Find Dependencies
    // Read local write
    match = find_match(depend_read_list, *depend_read_count, local_write_list, *write_count, r );
    if( match > 0)
    {
      wait(&status);
    }
    // Write local write
    match = find_match(depend_write_list, *depend_write_count, local_write_list, *write_count, r);
    if( match > 0)
    {
      wait(&status);
    }
    // Write local read
    match = find_match(depend_write_list, *depend_write_count, local_read_list, *read_count, r);
    if( match > 0 )
    {
      wait(&status);
    }

    pid_t pid;
    pid = fork();
    if (pid >= 0) // Fork successful
    {
      //Child
      if (pid == 0)
      {
        // Run Command
        recurse_command(c);

        // Remove Dependency from list
        remove_from_list(&depend_read_list, *depend_read_count, r);
        (*depend_read_count)--;
        remove_from_list(&depend_write_list, *depend_write_count, r);
        (*depend_write_count)--;

        exit(0); // Leave Process
      }
      // Parent
      else
      {
        return;
      }
    }
    else // Fork failed 
    {
      fprintf(stderr, "Fork Error");
      exit(1);
    }
    return;
  }
  else
  {
    recurse_command(c);
  }
  return;
}
