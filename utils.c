#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "pwm.h"
#include "md5.h"


int pwm_split_line
(char* string, char separator, char** fields, int max_fields) {
  char* pointer, *end_of_string, *aux;
  int count, len;
  len = strlen(string);
  if(len == 0) {
    return 0;
  }
  if (string[len-1] == '\n') {
    string[len-1] = '\0';
    len --;
  }
  pointer = string;
  end_of_string = string + len;
  count = 0;
  while (pointer < end_of_string && count < max_fields) {
    fields[count] = pointer;
    count++;
    aux = strchr(pointer, separator);
    if (aux != NULL) {
      *aux = '\0';
      pointer = aux + 1;
    } else {
      pointer = end_of_string;
    }
  }
  return count; 
}

pwm_res_t pwm_hash_password(salt_t salt, char* password, hash_t checksum) {
  MD5_CTX ctx;
  MD5Init(&ctx);
  MD5Update(&ctx, salt, sizeof(salt_t));
  MD5Update(&ctx, (unsigned char*) password, strlen(password));
  MD5Final(checksum, &ctx);
  return PWM_OK;
}

pwm_res_t pwm_generate_salt(salt_t salt) {
  FILE* fp = fopen("/dev/urandom", "r");
  pwm_res_t r = PWM_OK;
  if (!fp) {
    r = PWM_IO_ERROR;
  } else {
    if (fread(salt, sizeof(salt_t), 1, fp) != 1) { 
      r = PWM_IO_ERROR;
    }
    fclose(fp);
  }
  return r;
}

int pwm_decode_hex_string
(char* str, unsigned char* data, int expected_length) {
  unsigned char* pointer = (unsigned char*) str;
  int pos = 0;
  int hibits = 1;
  while (*pointer != '\0') {
    unsigned char v = *pointer;
    if (v >= '0' && v <= '9') {
      v -= '0';
    } else if (v >= 'a' && v <= 'f') {
      v = 10 + v - 'a';
    } else {
      break;
    }
    if (hibits) {
      data[pos] = v << 4;
    } else {
      data[pos] |= v;
      pos++;
    }
    hibits = !hibits;
    pointer++;
  }
  return pos == expected_length;
}

void pwm_print_hex_string(FILE* fp, unsigned char* data, int length) {
  int i;
  for (i = 0; i < length; i++) {
    fprintf(fp, "%02x", data[i]);
  }
}

void pwm_error(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  printf("PWM error! ");
  vfprintf(stdout, format, ap);
  putchar('\n');
  va_end(ap);
}
