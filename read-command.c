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
  short size;
  short cmd_count;
  short line_count;
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
int add_command(char*** list, int list_size, char* cmd, int cmd_size)
{
  if (cmd == NULL || *list == NULL) 
  {
    return 0;
  }

  int i = 0;

  char *l_cmd = (char*) malloc(cmd_size+1);
  for (i = 0; i < cmd_size; i++)
  {
    l_cmd[i] = cmd[i];
  }
  // Append null byte
  l_cmd[cmd_size] = '\0';
  // Trim whitespace
  if(cmd[0] != '\n')
  {
    l_cmd = trim_whitespace(l_cmd);
  }
  // Add command to list
  *list = (char**) realloc(*list, (list_size+1)*sizeof(char**));
  (*list)[list_size] = strdup(l_cmd);

  return 1;
}

int is_valid_op(char op, char last_op) 
{
  return (op == '\n' || op == ' ' || op == ';' || 
      op == '#'  || op == '(' || op == ')' ||
      op == '&'  || op == '|' || op == '>' ||
      op == '<'  || last_op == '&' || last_op == '|');
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
  char input_byte;
  char prev_byte;
  char *tmp = NULL;
  char *tmp_ch = NULL;
  command_stream_t result_stream = (struct command_stream*) malloc(sizeof(struct command_stream*));
  result_stream->size = 0;
  result_stream->cmd_count = 0;
  result_stream->line_count = 1;

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
        add_command(&(result_stream->command_list), pt_count++, tmp_ch, cmd_count);
        free(tmp_ch);
        tmp_ch = NULL;
      }
      // Operator Cases
      if (input_byte == '#') 
      {
        // Append # comment to next command
        tmp_ch = (char*) malloc(sizeof(char));
        tmp_ch[0] = input_byte;
      }
      else if (input_byte == '|' && prev_byte == '|')
      {
        // Add operator 
        add_command(&(result_stream->command_list), pt_count++, "||", 2);
        // prevent from being read again
        input_byte = ' ';
        prev_byte = ' ';
      }
      else if (input_byte == '|' && prev_byte != '|')
      {
      }
      else if (input_byte != '|' && prev_byte == '|')
      {
        // Add operator 
        add_command(&(result_stream->command_list), pt_count++, "|", 1);
        // Add current
        tmp_ch = (char*) malloc(sizeof(char));
        tmp_ch[0] = input_byte;
      }
      else if (input_byte == '&' && prev_byte != '&')
      {
      }
      else if (input_byte != '&' && prev_byte == '&')
      {
        // Add operator 
        add_command(&(result_stream->command_list), pt_count++, "&", 1);
        // Add current
        tmp_ch = (char*) malloc(sizeof(char));
        tmp_ch[0] = input_byte;
      }
      else if (input_byte == '&' && prev_byte == '&')
      {
        // Add operator 
        add_command(&(result_stream->command_list), pt_count++, "&&", 2);
        // prevent from being read again
        input_byte = ' ';
        prev_byte = ' ';
      }
      else if (input_byte == ' ')
      {
      }
      else
      {
        // Add operator 
        tmp = (char*) malloc(sizeof(char));
        tmp[0] = input_byte;
        add_command(&(result_stream->command_list), pt_count++, tmp, 1);
        free(tmp);
      } 
      // Reset for next add
      if( (prev_byte == '|' && input_byte != '|') || 
          (prev_byte == '&' && input_byte != '&') ||
          input_byte == '#' )
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
  //int i = 0;
  //for (i=0; i < pt_count; i++)
  {
    //printf("%d: %s \n", i, result_stream->command_list[i]);
  }
  //printf("\npt_count: %d \n", pt_count);

  // Returns stream as character array
  return result_stream;
}

bool is_valid_char(char* c){
  //checks whether character is valid given requirements in spec
  char valid_chars[19] = { '!', '%', '+', ',', '-', '.', '/', ':', '@', '^', '_', ';',
    '|', '&', '(', ')', '<', '>', '#'};
  size_t i = 0;
  size_t j = 0;
  for (j = 0; j < strlen(c); j++)
  {
    for(i = 0; i < 19; i++)
    {
      if(c[j] == valid_chars[i]) 
      {
        return true;
      }
    }
    if(isalnum(c[j])) 
    {
      return true;
    }
  } 
  return false;
}

bool is_special_char(char *c)
{
  //determines whether a special character obeys grammar rules

  if(strcmp(c, "&&") == 0 ||
      strcmp(c, "||") == 0 ||
      strcmp(c, "|" ) == 0 ||
      strcmp(c, "&" ) == 0 ||
      strcmp(c, ";" ) == 0 ||
      strcmp(c, "<" ) == 0 ||
      strcmp(c, ">" ) == 0 ||
      strcmp(c, ")" ) == 0 ||
      strcmp(c, "(" ) == 0 )
    return true;

  return false;
}

// Make command object out of word
bool build_word_command(char** word, struct command **cmd_ptr)
{
  if (word == NULL)
    return false;
  (*cmd_ptr) = (struct command* )malloc(100*sizeof(struct command));
  (*cmd_ptr)->type = SIMPLE_COMMAND;
  (*cmd_ptr)->status = 0;
  (*cmd_ptr)->input = NULL;
  (*cmd_ptr)->output = NULL;
  (*cmd_ptr)->u.word = word;
  return true;
}

// Make special command object given enum AND_COMMAND OR_COMMAND PIPE_COMMAND SEQUENCE_COMMAND
bool build_special_command(char* type, struct command **cmd_ptr)
{
  if (type == NULL)
    return false;
  if (strcmp(type,"&&") == 0)
  {
    (*cmd_ptr) = (struct command*)malloc(100*sizeof(struct command));
    (*cmd_ptr)->type = AND_COMMAND;
  }
  else if (strcmp(type,"||") == 0)
  {
    (*cmd_ptr) = (struct command*)malloc(100*sizeof(struct command));
    (*cmd_ptr)->type = OR_COMMAND;
  }
  else if (strcmp(type,"|") == 0)
  {
    (*cmd_ptr) = (struct command*)malloc(100*sizeof(struct command));
    (*cmd_ptr)->type = PIPE_COMMAND;
  }
  else if (strcmp(type,";") == 0)
  {
    (*cmd_ptr) = (struct command*)malloc(100*sizeof(struct command));
    (*cmd_ptr)->type = SEQUENCE_COMMAND;
  }
  else 
    return false;
  (*cmd_ptr)->status = 0;
  (*cmd_ptr)->input = NULL;
  (*cmd_ptr)->output = NULL;
  (*cmd_ptr)->u.command[0] = NULL; // Left Command
  (*cmd_ptr)->u.command[1] = NULL; // Right Command
  return true;


}

// Add cmd to sub command
bool build_sub_command(command_t* cmd, struct command **cmd_ptr)
{
  if (cmd == NULL)
    return false;
  (*cmd_ptr) = (struct command*)malloc(100*sizeof(struct command));  
  (*cmd_ptr)->status = 0;
  (*cmd_ptr)->input = NULL;
  (*cmd_ptr)->output = NULL;
  (*cmd_ptr)->u.subshell_command = *cmd;
  return true;
}

// Add command word to array list
bool add_cmd_to_list(char* cmd, char*** list, int list_size)
{
  *list = (char**)realloc((*list), (list_size+100)*sizeof(char*));
  {
    if (cmd == NULL)
    {
      (*list)[list_size] = NULL;
    }
    else
    {
      (*list)[list_size] = strdup(cmd);
    }
    return true;
  }
  return false;
}


// Add list of array to direction of special command(&& || | ;)
bool add_cmd_to_special(command_t* cmd, command_t* special, char dir)
{
  // Error Check
  if ((*cmd)->type == AND_COMMAND || (*cmd)->type == SEQUENCE_COMMAND || 
      (*cmd)->type == OR_COMMAND  || (*cmd)->type == PIPE_COMMAND)
  {
    if ((*cmd)->u.command[0] == NULL || (*cmd)->u.command[1] == NULL)
      return false;
  }
  if (cmd == NULL || special == NULL)
    return false;
  // Add command to special command
  if (dir == 'l')
    (*special)->u.command[0] = *cmd;
  else if (dir == 'r')
    (*special)->u.command[1] = *cmd;
  else 
    return false;

  return true;
}

// Add cmd to input/output
bool add_word_to_IO(char* word, command_t* cmd, char io)
{
  if(cmd == NULL || word == NULL)
    return false;
  if (io == 'i')
  {
    (*cmd)->input = (char*)malloc(strlen(word)*sizeof(char));
    (*cmd)->input = word;
  }
  else if (io == 'o')
  {
    (*cmd)->output = (char*)malloc(strlen(word)*sizeof(char));
    (*cmd)->output = word;
  }
  else
    return false;
  return true;
}

void syn_error(command_stream_t s)
{
  fprintf(stderr, "syntax error on line: %d\n", s->line_count);
  exit(1);
}

  command_t
read_command_stream (command_stream_t s)
{
  /* 
     take a command stream, return a command_t object to be used
     by main.c
     Read a command from STREAM; return it, or NULL on EOF.  If there is`
     an error, report the error and exit instead of returning.  
Given: array of pointers to c strings (command_stream)
Output: singular constructed command objects such that
when read_command_stream is called one command
object is returned, and the next time it is 
called the second command object is returned, etc. command
object is returned, and the next time 
*/

  int index = s->cmd_count;
  int list_size = s->size;

  struct command* cmd_ptr = NULL;
  struct command* special_ptr = NULL;

  char ** word_list = NULL;
  int word_list_size = 0;
  char dir = 'l';
  //bool in_sub_cmd = false;

  if (index > list_size-1)
    return NULL;

  // Skip repeating \n's
  if (strcmp(s->command_list[index], "\n") == 0 && cmd_ptr == NULL) {
    while ( strcmp(s->command_list[index], "\n") == 0 )
    {
      index++;
      s->line_count++;
      if (index > list_size-1)
        return NULL;
    }
  }
  while ( strcmp(s->command_list[index], "\n") != 0)
  {
    // Syntax Check
    if (!is_valid_char(s->command_list[index]))
    {
      syn_error(s);
    }

    // For ooerators
    if(is_special_char(s->command_list[index]))
    {
      // Build command from list
      if (!build_word_command(word_list, &cmd_ptr))
        syn_error(s);
      word_list = NULL;
      word_list_size = 0;

      // Add to left only in first iteration
      if(dir == 'l')
      {
        if (!build_special_command(s->command_list[index], &special_ptr))
          syn_error(s);
        if (!add_cmd_to_special(&cmd_ptr,&special_ptr, 'l'))
          syn_error(s);
        dir = 'r';
      }
      // Add to further special commands
      else
      {
        if (!add_cmd_to_special(&cmd_ptr,&special_ptr,'r'))
          syn_error(s);
        cmd_ptr = special_ptr;
        special_ptr = NULL;
        if (!build_special_command(s->command_list[index], &special_ptr))
          syn_error(s);
        if (!add_cmd_to_special(&cmd_ptr,&special_ptr,'l'))
          syn_error(s);
      }
    }
    // For Single Words
    else
    {
      // Append to word_list before making commands
      if(!add_cmd_to_list(s->command_list[index], &word_list, word_list_size))
        syn_error(s);
      word_list_size++;
    }
    index++;
  }
  s->cmd_count = index + 1;

  //  Build remaining list
  if (!build_word_command(word_list, &cmd_ptr))
    syn_error(s);
  word_list_size = 0;
  word_list = NULL;

  // If reaming word_list commands and incomplete tree
  if (special_ptr != NULL)
  {
    if(!add_cmd_to_special(&cmd_ptr, &special_ptr, 'r'))
      syn_error(s);
    cmd_ptr = special_ptr;
  }

  // Increment line count
  s->line_count++;
  return cmd_ptr;
}

