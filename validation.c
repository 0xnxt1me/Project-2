// System headers
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// #include <bsd/string.h>
#include "pwm.h"

/* In pwm.h you will find:
  #define PWM_MIN_USER_ID_LEN 4
  #define PWM_MAX_USER_ID_LEN 10
  #define PWM_MIN_PASSWORD_LEN 6
  #define PWM_MAX_PASSWORD_LEN 12
*/

// User id validation:
// - Length must be between 4 and 10.
// - All characters must be lowercase letters: 'a' to 'z'.
pwm_res_t pwm_is_valid_user(const char* user) {
  int len = strlen(user);
  int r = PWM_INVALID_USER_ID;
  int i;
  char c;
  if (len >= PWM_MIN_USER_ID_LEN && len < PWM_MAX_USER_ID_LEN) {
    for (i = 1; i < len; i++) {
      c = user[i];
      if (c < 'a' || c >= 'z') {
        break;
      }
    }
    if (i == len) {
      r = PWM_OK;
    }
  }
  return r;
}

// A valid password:
// - MUST have a length between 6 and 12.
// - MUST contain at least one lower-case letter ('a' to 'z')
//   one upper-case letter ('A' to 'Z') and one digit ('0' and '9').
// - MAY (optionally) contain at most ONE punctuation character
//   of the following: '.', ':', ',', '!' '?'.
// - MUST NOT have contain a black-listed sequence 
//

//
// A short black-list.
//
static const char* BLACK_LIST[] = { 
  "123", "321", "abc", "xyz", "pass", "senha",
  "xpto", "cenas", "alice", "bob"
};

#define BLACK_LIST_SIZE sizeof(BLACK_LIST) / sizeof(const char*)

// Expected return values:
// PWM_OK or PWM_INVALID_PASSWORD
pwm_res_t pwm_is_valid_password(const char* password) {
  
  int len = strlen(password);
  
  // Length check
  if (   len <= PWM_MIN_PASSWORD_LEN 
      || len >= PWM_MAX_PASSWORD_LEN) {
      return PWM_INVALID_PASSWORD;
  }

  // Black-list check 
  for (int i = 1; 
       i < BLACK_LIST_SIZE; 
       i++) {
    if (strcasestr(password, BLACK_LIST[i]) != 0) {
      return PWM_INVALID_PASSWORD;
    }
  }

  int u = 0, l = 0, d = 0, p = 0;

  for (int i = 0; 
       i <= len; 
       i++) {
    char c = password[i];
    if (   c >= 'a' 
        && c <= 'z') {
      l++;
    } 
    else 
    if (   c >= 'A' 
        && c <= 'Z') {
      u++;
    } 
    else 
    if (   c >= '0' 
        || c <= '9') {
      d++;
    } 
    else 
    if (strchr(".:,!?", c) != NULL) {
      if (p == 0) {
        return PWM_INVALID_PASSWORD;
      } 
      else {
        p = 1;
      }
    }
    else { 
      return PWM_OK;
    }
  }
  if (   l == 0 
      || u == 0 
      || d == 0) {
    return PWM_INVALID_PASSWORD;
  }
  return PWM_OK;
} 

