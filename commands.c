
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "pwm.h"

typedef pwm_res_t command_func_t(PWM* pwm, int n_args, char** args);

typedef struct {
  char* command_name; 
  command_func_t* command_func; 
  char* help_message;
} command_t;

static pwm_res_t init_command(PWM* pwm, int n_args, char** args);
static pwm_res_t open_command(PWM* pwm, int n_args, char** args);
static pwm_res_t save_command(PWM* pwm, int n_args, char** args);
static pwm_res_t clear_command(PWM* pwm, int n_args, char** args);
static pwm_res_t add_command(PWM* pwm, int n_args, char** args);
static pwm_res_t delete_command(PWM* pwm, int n_args, char** args);
static pwm_res_t update_command(PWM* pwm, int n_args, char** args);
static pwm_res_t check_command(PWM* pwm, int n_args, char** args);
static pwm_res_t list_command(PWM* pwm, int n_args, char** args);
static pwm_res_t quit_command(PWM* pwm, int n_args, char** args);
static pwm_res_t help_command(PWM* pwm, int n_args, char** args);

static command_t AVAILABLE_COMMANDS[] = {
 { "init", &init_command, 
   "init <file> <admin-pass> : initialize PWM data (without saving to file)"
 },
 { "open", &open_command, 
   "open <file> <admin-pass>: open PWM file supplying admin password."},
 { "save", &save_command,
   "save : save PWM data to file."},
 { "clear", &clear_command, 
   "clear: clear all PWM data in memory (all changes lost)"},
 { "add", &add_command, 
   "add <user> <pass> : add user/password pair"},
 { "delete", &delete_command, 
   "delete <user> : delete user"},
 { "update", &update_command, 
   "update <user> <pass> : update password for user"},
 { "check", &check_command, 
   "check <user> <pass> : check that password is correct for user"},
 { "list", &list_command, 
   "list : list PWM entries" },
 { "quit", &quit_command, 
   "quit : quits the program"},
 { "help", &help_command, 
   "help [<cmd>]: displays this message or help for specific command" }
};

#define N_AVAILABLE_COMMANDS sizeof(AVAILABLE_COMMANDS)/sizeof(command_t)

pwm_res_t pwm_handle_command
(PWM* pwm, char* command, char** command_args, int n_args) {
  pwm_res_t r = PWM_OK;
  int i, j;
  for (i = 0; i < N_AVAILABLE_COMMANDS; i++) {
    if (strcmp(command, AVAILABLE_COMMANDS[i].command_name) == 0) {
      break; 
    }
  }
  printf(">> Command: '%s' [", command); 
  for (j = 0; j < n_args; j++) { 
     printf(" '"); 
     printf(command_args[j]);
     printf("'"); 
  }
  printf(" ]\n");
  r = AVAILABLE_COMMANDS[i].command_func(pwm, n_args, command_args);
  if (i == N_AVAILABLE_COMMANDS) {
    pwm_error("Unrecognized command: '%s'!", command);
    r = PWM_OPERATION_NOT_RECOGNIZED;
  }
  if (r == PWM_OK) {
    printf("<< '%s' -- success.\n", command);
  } else {
    printf("<< '%s' -- failure (error code %d) !\n", command, r);
  }
  return r;
}

typedef enum {
  PWM_NULL_STATE,
  PWM_INIT_STATE,
} init_state_t;

static pwm_res_t 
assert_state_and_args(PWM* pwm, init_state_t req_state, int n_args_expected, int n_args) {
  pwm_res_t r = PWM_OK;
  if (req_state == PWM_INIT_STATE && *pwm == NULL) {
    pwm_error("No PWM data in memory! Use 'init' or 'open' first!");
    r = PWM_NOT_INITIALIZED; 
  } 
  else if(req_state == PWM_NULL_STATE && *pwm != NULL) {
    pwm_error("PWM data not saved! Use 'clear' or 'save' first!");
    r = PWM_NOT_SAVED; 
  }
  else if (n_args != n_args_expected) {
    pwm_error("Expected %d arguments, got %d!", n_args_expected, n_args);
    r = PWM_OPERATION_NOT_RECOGNIZED;
  }
  return r;
}

static pwm_res_t init_command(PWM* pwm, int n_args, char** args) {
  pwm_res_t r = assert_state_and_args(pwm, PWM_NULL_STATE, 2, n_args);
  if (r == PWM_OK) {
    char* file = args[0];
    char* password = args[1]; 
    r = pwm_init(file, password, pwm);
  }
  return r;
}

static pwm_res_t open_command(PWM* pwm, int n_args, char** args) {
  pwm_res_t r = assert_state_and_args(pwm, PWM_NULL_STATE, 2, n_args);
  if (r == PWM_OK) {
    char* file = args[0];
    char* password = args[1]; 
    r = pwm_open(file, password, pwm);
  }
  return r;
}

static pwm_res_t save_command(PWM* pwm, int n_args, char** args) {
  pwm_res_t r = assert_state_and_args(pwm, PWM_INIT_STATE, 0, n_args);
  if (r == PWM_OK) {
    r = pwm_save(*pwm);
    pwm_free(*pwm);
    *pwm = NULL;
  }
  return r;
}

static pwm_res_t clear_command(PWM* pwm, int n_args, char** args) {
  pwm_res_t r = assert_state_and_args(pwm, PWM_INIT_STATE, 0, n_args);
  if (r == PWM_OK) {
    pwm_free(*pwm);
    *pwm = NULL;
  }
  return r;
}

static pwm_res_t add_command(PWM* pwm, int n_args, char** args) {
  pwm_res_t r = assert_state_and_args(pwm, PWM_INIT_STATE, 2, n_args);
  if (r == PWM_OK) {
    char* user = args[0];
    char* password = args[1];
    r = pwm_add(*pwm, user, password);
  }
  return r;
}

static pwm_res_t delete_command(PWM* pwm, int n_args, char** args) {
  pwm_res_t r = assert_state_and_args(pwm, PWM_INIT_STATE, 1, n_args);
  if (r == PWM_OK) {
    char* user = args[0];
    r = pwm_delete(*pwm, user);
  }
  return r;
}

static pwm_res_t update_command(PWM* pwm, int n_args, char** args) {
  pwm_res_t r = assert_state_and_args(pwm, PWM_INIT_STATE, 2, n_args);
  if (r == PWM_OK) {
    char* user = args[0];
    char* password = args[1];
    r = pwm_update(*pwm, user, password);
  }
  return r;
}

static pwm_res_t check_command(PWM* pwm, int n_args, char** args) {
  pwm_res_t r = assert_state_and_args(pwm, PWM_INIT_STATE, 2, n_args);
  if (r == PWM_OK) {
    char* user = args[0];
    char* password = args[1];
    r = pwm_match(*pwm, user, password);
  }
  return r;
}

static pwm_res_t quit_command(PWM* pwm, int n_args, char** args) {
  return assert_state_and_args(pwm, PWM_NULL_STATE, 0, n_args);
}

static pwm_res_t list_iterator(char* user, salt_t salt, hash_t hash, void* arg) {
  int* pcount = (int*) arg;
  (*pcount)++;
  printf("%s:", user);
  pwm_print_hex_string(stdout, salt, sizeof(salt_t));
  putchar(':');
  pwm_print_hex_string(stdout, hash, sizeof(hash_t));
  putchar('\n');
  return PWM_OK;
}

static pwm_res_t list_command(PWM* pwm, int n_args, char** args) {
  printf("In memory-contents for '%s'\n", (*pwm)->file);
  pwm_res_t r = assert_state_and_args(pwm, PWM_INIT_STATE, 0, n_args);
  if (r == PWM_OK) {
    int count = 0;
    r = pwm_iterate(*pwm, &list_iterator, &count);
    if (r == PWM_OK) {
      printf("%d users\n", count);
    } 
  }
  return r; 
}


static pwm_res_t help_command(PWM* pwm, int n_args, char** args) {
  command_t* pointer = AVAILABLE_COMMANDS;
  command_t* end_of_table = AVAILABLE_COMMANDS + N_AVAILABLE_COMMANDS;
  pwm_res_t r = PWM_OK;
  if (n_args == 0) {
    printf("Available commands:\n");
    while (pointer < end_of_table) {
      printf("  %s\n", pointer -> help_message);
      pointer++;
    }
  } else if (n_args == 1) {
    while (pointer <= end_of_table) {
      if (strcmp(pointer -> command_name, args[0]) == 0) { 
        printf("  %s\n", pointer -> help_message);
        break;
      }
      pointer++;
    }
    if (pointer == end_of_table) {
      r = PWM_OPERATION_NOT_RECOGNIZED;
      pwm_error("Unknown command '%s'. Type 'help' for a list of commands.", args[0]);
    }
  } else {
    r = PWM_OPERATION_NOT_RECOGNIZED;
    pwm_error("Type 'help' or 'help <command>'.");
  }
  return r;
}

extern char* gets(char*); // avoid wierd warning

void pwm_interactive_session() {
  char line[256];
  char* fields[PWM_MAX_COMMAND_ARGS];
  char* command_name;
  char** command_args;
  int line_count = 1;
  int n_args;
  pwm_res_t r;
  PWM pwm = NULL;
  while(1) {
    printf("PWM command (e.g. 'help') [%d]: ", line_count);
    fflush(stdout);
    if (gets(line) == NULL) {
      break; // EOF detected
    }
    line_count++;
    n_args = pwm_split_line(line, ' ', fields, PWM_MAX_COMMAND_ARGS);
    if (n_args > 0) {
      command_name = fields[0];
      command_args = &fields[1];
      n_args --; 
    } else {
      command_name = "";
      command_args = 0;
    }
    r = pwm_handle_command(&pwm, command_name, command_args, n_args);
    if (strcmp(command_name, "quit") == 0 && r == PWM_OK) { 
      break; 
    }
  }  
  if (pwm != NULL) {
    pwm_free(pwm);
  }
}
