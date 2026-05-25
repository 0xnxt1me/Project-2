#ifndef __pwm_h__
#define __pwm_h__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>

// Error codes
typedef enum 
{
  PWM_OK,
  PWM_NOT_INITIALIZED,
  PWM_NOT_SAVED,
  PWM_OPERATION_NOT_ALLOWED,
  PWM_OPERATION_NOT_RECOGNIZED,
  PWM_USER_NOT_FOUND,
  PWM_USER_ALREADY_EXISTS,
  PWM_INVALID_USER_ID,
  PWM_INVALID_PASSWORD,
  PWM_PASSWORD_MISMATCH,
  PWM_FILE_INACESSIBLE,
  PWM_FILE_ALREADY_EXISTS,
  PWM_FILE_CORRUPT,
  PWM_IO_ERROR,
  PWM_OS_ERROR,
  PWM_MEMORY_ALLOCATION_ERROR
} pwm_res_t;

// User and password validation
#define PWM_MIN_USER_ID_LEN 4
#define PWM_MAX_USER_ID_LEN 10
#define PWM_MIN_PASSWORD_LEN 6
#define PWM_MAX_PASSWORD_LEN 12

// Validation functions
pwm_res_t pwm_is_valid_user(const char* user);
pwm_res_t pwm_is_valid_password(const char* password);

// Global definitions for salt and hash
typedef unsigned char salt_t[4];
typedef unsigned char hash_t[16];

// Admin user name
#define PWM_ADMIN_USER "admin"

// Data structures for internal representation
typedef struct _pwm_node_t {
  char user[PWM_MAX_USER_ID_LEN + 1];
  salt_t salt;
  hash_t hash;
  struct _pwm_node_t* next;
} pwm_node_t;

struct _pwm_t {
   char* file;
   pwm_node_t* entries;
};
typedef struct _pwm_t* PWM;


// Core operations
pwm_res_t pwm_init(char* file, char* password, PWM* pwm);
pwm_res_t pwm_open(char* file, char* password, PWM* pwm);
pwm_res_t pwm_save(PWM pwm);
pwm_res_t pwm_free(PWM pwm);

pwm_res_t pwm_match(PWM pwm, char *user, char* password);
pwm_res_t pwm_add(PWM pwm, char* user, char* password);
pwm_res_t pwm_update(PWM pwm, char *user, char* password);
pwm_res_t pwm_delete(PWM pwm, char* user); 

typedef pwm_res_t pwm_iterator_t(char* user, salt_t salt, hash_t hash, void* arg);
pwm_res_t pwm_iterate(PWM pwm, pwm_iterator_t* iterator, void* arg);

// Command handling
#define PWM_MAX_COMMAND_ARGS 3

pwm_res_t pwm_handle_command
(PWM* pwm, char* command_name, char** args, int n_args);

// Interactive session 
void pwm_interactive_session(void);


// UTILITY FUNCTIONS
pwm_res_t pwm_generate_salt(salt_t salt);
pwm_res_t pwm_hash_password(salt_t salt, char* password, hash_t hash);
int pwm_decode_hex_string(char* str, unsigned char* data, int expected_length);
void pwm_print_hex_string(FILE* fp, unsigned char* data, int length);

int pwm_split_line
(char* string, char separator, char** fields, int max_fields);

void pwm_error(const char* fmt, ...);

#ifdef __cplusplus
}
#endif
#endif


