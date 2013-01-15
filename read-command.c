// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <ctype.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */

typedef struct command_stream 
{
  // Array of pointers pointing to c-strings
  int size;
  int cmd_count;
  char** command_list;
} command_stream;

// Trim whitespace off commands
char* trim_whitespace(char* str)
{
  char *end;
  while(isspace(*str)) str++;
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;
  *(end+1) = 0;
  return str;
}

// Add command to command stream
int add_command(char*** list, int cur_size, char** cmd, int len)
{
  if (*cmd == NULL || *list == NULL) 
  {
    return 0;
  }
  // Append null byte
  *cmd = (char*) realloc(*cmd, (len+1)*sizeof(char));
  (*cmd)[len] = '\0';
  // Trim whitespace
  *cmd = trim_whitespace(*cmd);
  // Add command to list
  *list = (char**) realloc(*list, (cur_size+1)*sizeof(char**));
  (*list)[cur_size] = malloc(strlen(*cmd));
  (*list)[cur_size] = strdup(*cmd);
  *cmd = NULL;
  return 1;
}

int is_valid_op(char op, char last_op) 
{
  return (op == '\n' || op == ';' || op == '#' || op == '(' || op == ')' ||
      op == '&'  || op == '|' || last_op == '&' || last_op == '|');
}

  command_stream_t
make_command_stream (int (*get_next_byte) (void *),
    void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */

  // initialize memory allocation
  // count of characters
  int pt_count = 0;
  int cmd_count = 0;
  int i = 0;
  char input_byte;
  char prev_byte;
  char *tmp = NULL;
  char *tmp_ch = NULL;
  command_stream t = { 0, 0, NULL };
  command_stream_t result_stream = &t;

  // Initalize memory allocation for stream
  result_stream->command_list = (char**) malloc(sizeof(char**));
  result_stream->command_list[0] = NULL;

  // initialize stream
  input_byte = (char)get_next_byte(get_next_byte_argument);

  // Reallocate memory for stream characters
  // Record stream
  while (input_byte != EOF)
  {    
    if (is_valid_op(input_byte, prev_byte))
    {
      // Add command
      if (tmp_ch != NULL) 
      {
        add_command(&(result_stream->command_list), pt_count++, &tmp_ch, cmd_count);
      }
      // Operator Cases
      if (input_byte == '#') 
      {
        // Append # comment to next command
        tmp_ch = (char*) malloc(sizeof(char));
        tmp_ch[0] = input_byte;
      }
      else if (input_byte == '|' && prev_byte != '|')
      {
      }
      else if (input_byte != '|' && prev_byte == '|')
      {
        // Add operator 
        tmp = (char*) malloc(sizeof(char));
        tmp[0] = prev_byte;
        add_command(&(result_stream->command_list), pt_count++, &tmp, 1);
      }
      else if (input_byte == '|' && prev_byte == '|')
      {
        // Add operator 
        tmp = (char*) malloc(2*sizeof(char));
        tmp[0] = input_byte;
        tmp[1] = prev_byte;
        add_command(&(result_stream->command_list), pt_count++, &tmp, 2);
        // prevent from being read again
        input_byte = ' ';
      }
      else if (input_byte == '&' && prev_byte != '&')
      {
      }
      else if (input_byte != '&' && prev_byte == '&')
      {
        // Add operator 
        tmp = (char*) malloc(sizeof(char));
        tmp[0] = input_byte;
        add_command(&(result_stream->command_list), pt_count++, &tmp, 1);
      }
      else if (input_byte == '&' && prev_byte == '&')
      {
        // Add operator 
        tmp = (char*) malloc(2*sizeof(char));
        tmp[0] = input_byte;
        tmp[1] = prev_byte;
        add_command(&(result_stream->command_list), pt_count++, &tmp, 2);
        // prevent from being read again
        input_byte = ' ';
      }
      else
      {
        // Add operator 
        tmp = (char*) malloc(sizeof(char));
        tmp[0] = input_byte;
        add_command(&(result_stream->command_list), pt_count++, &tmp, 1);
      }

      // Reset for next add
      if(input_byte == '#')
      {
        cmd_count = 1;
      }
      else
      {
        cmd_count = 0;
      }
      prev_byte = input_byte;
      input_byte = get_next_byte(get_next_byte_argument);
      continue;
    }
    if (input_byte == '#') 
    {
      // Reset
      prev_byte = input_byte;
      input_byte = get_next_byte(get_next_byte_argument);
      continue;
    }
    // Allocate for command until delimiter found
    tmp_ch = (char*) realloc(tmp_ch, (cmd_count+1)*sizeof(char));
    tmp_ch[cmd_count] = input_byte;
    cmd_count++;

    prev_byte = input_byte;
    input_byte = get_next_byte(get_next_byte_argument);
  }

  result_stream->size = pt_count;

  // Test print
  for (i=0; i < pt_count; i++)
  {
    printf("%d: %s\n", i, result_stream->command_list[i]);
  }
  printf("\npt_count: %d \n", pt_count);

  // Returns stream as character array
  return result_stream;
}

bool isValidChar(char c){
  //checks whether character is valid given requirements in spec
  char valid_chars[19] = { '!', '%', '+', ',', '-', '.', '/', ':', '@', '^', '_', ';',
    '|', '&', '(', ')', '<', '>', '#'};
  size_t i = 0;
  for(i = 0; i < strlen(valid_chars); i++){
    if(c != valid_chars[i]) return false;
  }

  if(!isalnum(c)) return false;
  return true;
}

bool noSpecialChar(char *c)
{
  //determines whether a special character obeys grammar rules
  if(strcmp(c, "&&")==0) return false;
  if(strcmp(c, "||")==0) return false;	
  if(strcmp(c, "|")==0) return false;
  if(strcmp(c, "&")==0) return false;
  if(strcmp(c, ";")==0) return false;
  if(strcmp(c, "<")==0) return false;
  if(strcmp(c, ">")==0) return false;
  if(strcmp(c, ")")==0) return false;
  if(strcmp(c, "(")==0) return false;

  return true;
}

  command_t
read_command_stream (command_stream_t s)
{
  /* 
     take a command stream, return a command_t object to be used
     by main.c
     Read a command from STREAM; return it, or NULL on EOF.  If there is
     an error, report the error and exit instead of returning.  
Given: array of pointers to c strings (command_stream)
Output: singular constructed command objects such that
when read_command_stream is called one command
object is returned, and the next time it is 
called the second command object is returned, etc.
*/
  size_t i = 0;
  int index = s->cmd_count;
  int list_size = s->size;

  //if the next command exists
  if(index != list_size)
  {
    command_t cmd_ptr = NULL;
    cmd_ptr = (struct command*) malloc(sizeof(struct command));

        //check for special tokens:
    if(strcmp("&&", s->command_list[index])==0)
    {
      //if there is another command before and afterwards
      if(index+1 <= list_size-1 && index-1 >=0 )
      {
        if(noSpecialChar(s->command_list[index+1]))
        {
          cmd_ptr->type = AND_COMMAND;
          cmd_ptr->status = 0;
          struct command *previous = NULL;
          struct command *next = NULL;
          previous->u.word = &(s->command_list[index-1]);
          next->u.word = &(s->command_list[index+1]);
          cmd_ptr->u.command[0] = previous;
          cmd_ptr->u.command[1] = next; //am i doing this right?
          //construct 
        }
        else
        {
          //print an error message
        } 	
      }
      else
      {
        //print another error message
      }

    } 
    else if(strcmp("||", s->command_list[index])==0)
    {
      if(index+1 <= list_size-1 && index-1 >=0 )
      {
        if(noSpecialChar(s->command_list[index+1]))
        {
          cmd_ptr->type = OR_COMMAND;
          cmd_ptr->status = 0;
          struct command *previous = NULL;
          struct command *next = NULL;
          previous->u.word = &(s->command_list[index-1]);
          next->u.word = &(s->command_list[index+1]);
          cmd_ptr->u.command[0] = previous;
          cmd_ptr->u.command[1] = next; //am i doing this right?
          //construct 
        }

        else
        {
        }
      } 
      else
      {
      }
    }
    else if(strcmp(";", s->command_list[index])==0)
    {
      if(index+1 <= list_size-1 && index-1 >=0 )
      {
        if(noSpecialChar(s->command_list[index+1]))
        {
          cmd_ptr->type = SEQUENCE_COMMAND;
          cmd_ptr->status = 0;
          struct command *previous = NULL;
          struct command *next = NULL;
          previous->u.word = &(s->command_list[index-1]);
          next->u.word = &(s->command_list[index+1]);
          cmd_ptr->u.command[0] = previous;
          cmd_ptr->u.command[1] = next; 
        }				
        else
        {
        }
      }
      else
      {	
      }
    }
    else if(strcmp("|", s->command_list[index])==0)
    {
      if(index+1 <= list_size-1 && s->cmd_count-1 >=0 )
      {
        if(noSpecialChar(s->command_list[index+1]))
        {
          cmd_ptr->type = PIPE_COMMAND;
          cmd_ptr->status = 0;
          struct command *previous = NULL;
          struct command *next = NULL;
          previous->u.word = &(s->command_list[index-1]);
          next->u.word = &(s->command_list[index+1]);
          cmd_ptr->u.command[0] = previous;
          cmd_ptr->u.command[1] = next; 
        }				
        else
        {
        }
      }
    }
    else if(strcmp("(", s->command_list[index])==0)
    {
      if(index+1 <= list_size-1)
      {
        if(noSpecialChar(s->command_list[index+1]))
        {
          cmd_ptr->type = SUBSHELL_COMMAND;
          cmd_ptr->status = 0;

          //cmd_ptr->u.subshell_command = s->command_list[index];
        }
      }
    }
    else if(strcmp(")", s->command_list[index])==0)
    {

    }
    else
    {
      for(i = 0; i < strlen(s->command_list[index]); i++)
      {
        if(!isValidChar(s->command_list[index][i]))
        {
          //print error, abort
        }
      }
      //valid command; execute
    }

    s->cmd_count += 1;
    cmd_ptr->status = 0;
    cmd_ptr->input = NULL;
    cmd_ptr->output = NULL;
    cmd_ptr->type = SIMPLE_COMMAND;
    char** tmp2  = (char**) malloc(sizeof(char**));
    tmp2[0] = (char*) malloc(10*sizeof(char));
    for(i = 0; i < 4; i++)
    {
      tmp2[0][i] = 'a';
    }
    tmp2[4] = '\0';
    cmd_ptr->u.word = tmp2;

    return cmd_ptr;
  }
  else
  {
    //check for matching number of left and right 
    //parentheses

    return NULL;
  }
}

