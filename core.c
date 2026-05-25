// System headers
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
// #include <bsd/string.h>

// PWM header
#include "pwm.h"

static pwm_node_t* create_node(char* user, salt_t salt, hash_t hash) {
  pwm_node_t* node = (pwm_node_t*) malloc(sizeof(pwm_node_t));
  strcpy(node -> user, user);
  memcpy(node -> salt, salt, sizeof(node -> salt));
  memcpy(node -> hash, hash, sizeof(node -> hash));
  node -> next = NULL;
  return node;
}

static pwm_res_t create_node_p(char* user, char* password, pwm_node_t** node) {
   salt_t salt;
   hash_t hash;
   pwm_res_t r = PWM_OK;
   if ( (r = pwm_generate_salt(salt)) == PWM_OK
       && (r = pwm_hash_password(salt, password, hash)) == PWM_OK) {
      *node = create_node(user, salt, hash);
   }
   return r;
}


static pwm_res_t pwm_alloc(char* file, PWM* p_pwm) {
  PWM pwm = (PWM) malloc(sizeof(struct _pwm_t));
  if (pwm == NULL) {
    perror("malloc");
    pwm_error("Could not allocate memory!");
    return PWM_MEMORY_ALLOCATION_ERROR;
  }
  pwm -> file = strdup(file);
  if (pwm -> file == NULL) {
    free(pwm);
    perror("strdup");
    pwm_error("Could not allocate memory!");
    free(pwm);
    return PWM_MEMORY_ALLOCATION_ERROR;
  }
  pwm -> entries = NULL;
  *p_pwm = pwm;
  return PWM_OK;
}

pwm_res_t pwm_free(PWM pwm) {
  pwm_node_t* node = pwm -> entries;
  while (node != NULL) {
    pwm_node_t* aux = node;
    node = node -> next;
    free(aux);
  }
  free(pwm);
  return PWM_OK;
}

pwm_res_t pwm_init(char* file, char* password, PWM* pwm) {
  pwm_res_t r = PWM_OK;
  if ((r = pwm_is_valid_password(password)) != PWM_OK) {
    pwm_error("Invalid password '%s'!", password);
    return r;
  }
  if ((r = pwm_alloc(file, pwm)) != PWM_OK) {
    return r;
  }
  return create_node_p(PWM_ADMIN_USER, password, &((*pwm) -> entries));
}

pwm_res_t pwm_open(char* file, char* password, PWM* pwm) {
  pwm_res_t r = PWM_OK;
  FILE* f = fopen(file, "r");
  if (f == NULL) {
    *pwm = NULL;
    r = PWM_FILE_INACESSIBLE;
    perror(file);
    pwm_error("Could not open file '%s' for reading!", file);
  } else if ((r = pwm_alloc(file, pwm)) == PWM_OK) {
     char line[1024];
     int count = 0;
     pwm_node_t* curr;
     while (fgets(line, sizeof(line), f) != NULL) {
       char* line_fields[3];
       char* user, *salt_str, *hash_str;
       salt_t salt; hash_t hash;
       pwm_node_t* node;
       count++;
       if (pwm_split_line(line, ':', line_fields, 3) != 3) {
         pwm_error("Corrupt file '%s' at line %d [entries] !", (*pwm)->file, count+1);
         r = PWM_FILE_CORRUPT;
         break;
       }
       user = line_fields[0];
       salt_str = line_fields[1];
       hash_str = line_fields[2];
       if (! pwm_decode_hex_string(salt_str, salt, sizeof(salt_t))) {
         pwm_error("Corrupt file '%s' at line %d [salt] !", (*pwm)->file, count+1);
         r = PWM_FILE_CORRUPT;
         break;
       }
       if (! pwm_decode_hex_string(hash_str, hash, sizeof(hash_t))) {
         pwm_error("Corrupt file '%s' at line %d [hash] !", (*pwm)->file, count+1);
         r = PWM_FILE_CORRUPT;
         break;
       }
       node = create_node(user, salt, hash);
       if (count == 1) {
         hash_t vhash;
         pwm_hash_password(node -> salt, password, vhash);
         if (memcmp(node -> hash, vhash, sizeof(hash_t)) != 0) {
           r = PWM_PASSWORD_MISMATCH;
           pwm_error("Password mismatch for admin user!");
           break;
         }
         (*pwm) -> entries = node;
       } else {
         curr -> next = node;
       }  
       curr = node;
     }
     if (r != PWM_OK) {
       pwm_free(*pwm); 
       *pwm = NULL;
     }
     fclose(f);
  }
  return r; 
}

pwm_res_t pwm_update(PWM pwm, char* user, char* password) {
  int r = PWM_OK;
  if ((r = pwm_is_valid_password(password)) == PWM_OK) {
    pwm_node_t* node = pwm -> entries;
    while (node != NULL) {
      if ( strcmp(node -> user, user) == 0) {
        salt_t salt;
        hash_t hash;
        if ( (r = pwm_generate_salt(salt)) == PWM_OK
           && (r = pwm_hash_password(salt, password, hash)) == PWM_OK) {
          memcpy(node -> salt, salt, sizeof(salt));
          memcpy(node -> hash, hash, sizeof(hash));
        }
        break;
      }
      node = node -> next;
    }
    if (node == NULL) {
      r = PWM_USER_NOT_FOUND;
      pwm_error("User '%s' does not exist!", user);
    }  
  } else {
    pwm_error("Invalid password '%s'!", password);
  }
  return r;
}

pwm_res_t pwm_add(PWM pwm, char* user, char* password) {
  pwm_res_t r = PWM_OK;
  if ( strcmp(user, PWM_ADMIN_USER) == 0) {
    r = PWM_OPERATION_NOT_ALLOWED;
    pwm_error("User '%s' cannot be added!", user);
  }
  else if ((r = pwm_is_valid_user(user)) != PWM_OK) {
    pwm_error("Invalid user id '%s'!", user);
  } else if ((r = pwm_is_valid_password(password)) != PWM_OK) {
    pwm_error("Invalid password '%s'!", password);
  } else {
    pwm_node_t* node = pwm -> entries;
    while (node -> next != NULL) {
      if (strcmp(node -> next -> user, user) == 0) {
        pwm_error("User '%s' already exists!", user);
        r = PWM_USER_ALREADY_EXISTS;
        break;
      }
      node = node -> next;
    }
    if (node -> next == NULL) {
      r = create_node_p(user, password, & (node -> next));
    }
  }
  return r;
}

pwm_res_t pwm_delete(PWM pwm, char* user) {
  pwm_res_t r = PWM_OK;
  if ( strcmp(user, PWM_ADMIN_USER) == 0) {
    pwm_error("User '%s' cannot be deleted!", user);
    r = PWM_OPERATION_NOT_ALLOWED;
  } else {
    pwm_node_t* node = pwm -> entries;
    while (node -> next != NULL) {
      if (strcmp(node -> next -> user, user) == 0) {
        break;
      }
      node = node -> next;
    }
    if (node -> next == NULL) {
      r = PWM_USER_NOT_FOUND;
      pwm_error("User '%s' does not exist!", user);
    } else {
      free(node -> next);
      node -> next = node -> next -> next;
    } 
  }
  return r;
}

pwm_res_t pwm_save(PWM pwm) {
  FILE* f = fopen(pwm -> file, "w");
  if (!f) {
    perror(pwm -> file);
    pwm_error("Could not open file '%s' for writing!", pwm -> file);
    return PWM_IO_ERROR;
  }
  pwm_node_t* node = pwm -> entries;
  while (node != NULL) {
    fprintf(f, "%s:", node -> user);
    pwm_print_hex_string(f, node -> salt, sizeof(salt_t));
    fputc(':', f);
    pwm_print_hex_string(f, node -> hash, sizeof(hash_t));
    fputc('\n', f);
    node = node -> next;
  } 
  fclose(f);
  return PWM_OK; 
}

pwm_res_t pwm_iterate(PWM pwm, pwm_iterator_t* iterator, void* arg) {
  pwm_res_t r = PWM_OK;
  pwm_node_t* node = pwm -> entries;
  while (node != NULL && r == PWM_OK) {
    r = iterator(node -> user, node -> salt, node -> hash, arg);
    node = node -> next;
  }
  return r;
}

pwm_res_t pwm_match(PWM pwm, char* user, char* password) {
  pwm_node_t* node = pwm -> entries;
  pwm_res_t r = PWM_OK;
  hash_t hash;
  while (node != NULL) {
    if ( strcmp(node -> user, user) == 0) {
      break;
    }
    node = node -> next;
  }
  if (node == NULL) {
    pwm_error("User '%s' does not exist!", user);
    r = PWM_USER_NOT_FOUND;
  } 
  else 
  if ((r = pwm_hash_password(node -> salt, password, hash)) == PWM_OK) {
    if (memcmp(node -> hash, hash, sizeof(hash_t)) != 0) {
      pwm_error("Password mismatch for user '%s'!", user);
      r = PWM_PASSWORD_MISMATCH; 
    }
  }  
  return r;
}

