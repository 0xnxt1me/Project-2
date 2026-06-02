// System headers
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
// #include <bsd/string.h>

// PWM header
#include "pwm.h"

#include <openssl/evp.h>
#include <openssl/rand.h>

#define AEAD_MAGIC "PWMAEAD"
#define AEAD_MAGIC_LEN 8
#define KDF_SALT_LEN 16
#define GCM_IV_LEN 12
#define GCM_TAG_LEN 16
#define KDF_ITERATIONS 10000

static int aes_gcm_encrypt(const unsigned char *plaintext, int plaintext_len,
                           const unsigned char *key, const unsigned char *iv,
                           unsigned char *ciphertext, unsigned char *tag) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;

    if (!(ctx = EVP_CIPHER_CTX_new())) return -1;
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)) goto err;
    if (1 != EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv)) goto err;

    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)) goto err;
    ciphertext_len = len;

    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) goto err;
    ciphertext_len += len;

    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, GCM_TAG_LEN, tag)) goto err;

    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;

err:
    EVP_CIPHER_CTX_free(ctx);
    return -1;
}

static int aes_gcm_decrypt(const unsigned char *ciphertext, int ciphertext_len,
                           const unsigned char *tag, const unsigned char *key,
                           const unsigned char *iv, unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;
    int ret;

    if (!(ctx = EVP_CIPHER_CTX_new())) return -1;
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)) goto err;
    if (1 != EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv)) goto err;

    if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)) goto err;
    plaintext_len = len;

    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, GCM_TAG_LEN, (void*)tag)) goto err;

    ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);

    EVP_CIPHER_CTX_free(ctx);

    if (ret > 0) {
        plaintext_len += len;
        return plaintext_len;
    } else {
        return -1;
    }

err:
    EVP_CIPHER_CTX_free(ctx);
    return -1;
}

static pwm_node_t* create_node(char* user, salt_t salt, hash_t hash) {
  pwm_node_t* node = (pwm_node_t*) malloc(sizeof(pwm_node_t));
  strlcpy(node -> user, user, sizeof(node -> user));
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
       && (r = pwm_hash_password(user, salt, password, hash)) == PWM_OK) {
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

  memset(pwm->admin_password, 0, sizeof(pwm->admin_password));

  if (pwm -> file != NULL) {
    free(pwm -> file);
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
  strlcpy((*pwm)->admin_password, password, sizeof((*pwm)->admin_password));
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
    return r;
  }

  if ((r = pwm_alloc(file, pwm)) != PWM_OK) {
    fclose(f);
    return r;
  }
  strlcpy((*pwm)->admin_password, password, sizeof((*pwm)->admin_password));

  char magic[AEAD_MAGIC_LEN];
  size_t bytes_read = fread(magic, 1, AEAD_MAGIC_LEN, f);
  
  FILE* parse_f = NULL;
  char* decrypted_plaintext = NULL;

  if (bytes_read == AEAD_MAGIC_LEN && memcmp(magic, AEAD_MAGIC, AEAD_MAGIC_LEN) == 0) {
    unsigned char kdf_salt[KDF_SALT_LEN];
    unsigned char iv[GCM_IV_LEN];
    unsigned char tag[GCM_TAG_LEN];
    
    if (fread(kdf_salt, 1, KDF_SALT_LEN, f) != KDF_SALT_LEN ||
        fread(iv, 1, GCM_IV_LEN, f) != GCM_IV_LEN ||
        fread(tag, 1, GCM_TAG_LEN, f) != GCM_TAG_LEN) {
      pwm_error("Corrupt encrypted database metadata!");
      r = PWM_FILE_CORRUPT;
      goto decrypt_fail;
    }

    long meta_len = AEAD_MAGIC_LEN + KDF_SALT_LEN + GCM_IV_LEN + GCM_TAG_LEN;
    fseek(f, 0, SEEK_END);
    long file_len = ftell(f);
    long ciphertext_len = file_len - meta_len;
    if (ciphertext_len <= 0) {
      pwm_error("Corrupt encrypted database ciphertext!");
      r = PWM_FILE_CORRUPT;
      goto decrypt_fail;
    }
    fseek(f, meta_len, SEEK_SET);

    unsigned char* ciphertext = malloc(ciphertext_len);
    if (fread(ciphertext, 1, ciphertext_len, f) != ciphertext_len) {
      pwm_error("Failed to read ciphertext!");
      free(ciphertext);
      r = PWM_FILE_CORRUPT;
      goto decrypt_fail;
    }

    unsigned char key[32];
    if (1 != PKCS5_PBKDF2_HMAC(password, strlen(password),
                               kdf_salt, KDF_SALT_LEN, KDF_ITERATIONS,
                               EVP_sha256(), 32, key)) {
      pwm_error("PBKDF2 key derivation failed!");
      free(ciphertext);
      r = PWM_OS_ERROR;
      goto decrypt_fail;
    }

    decrypted_plaintext = malloc(ciphertext_len + 1);
    int decrypted_len = aes_gcm_decrypt(ciphertext, ciphertext_len, tag, key, iv, (unsigned char*)decrypted_plaintext);
    
    memset(key, 0, sizeof(key));
    free(ciphertext);

    if (decrypted_len < 0) {
      pwm_error("Decryption failed! Password mismatch or database tampered!");
      r = PWM_PASSWORD_MISMATCH;
      goto decrypt_fail;
    }
    decrypted_plaintext[decrypted_len] = '\0';

    parse_f = fmemopen(decrypted_plaintext, decrypted_len, "r");
    if (!parse_f) {
      pwm_error("fmemopen failed!");
      r = PWM_MEMORY_ALLOCATION_ERROR;
      goto decrypt_fail;
    }
  } else {
    fseek(f, 0, SEEK_SET);
    parse_f = f;
  }

  char line[1024];
  int count = 0;
  pwm_node_t* curr = NULL;
  while (fgets(line, sizeof(line), parse_f) != NULL) {
    char* line_fields[3];
    char* user, *salt_str, *hash_str;
    salt_t salt; hash_t hash;
    pwm_node_t* node;
    count++;
    if (pwm_split_line(line, ':', line_fields, 3) != 3) {
      pwm_error("Corrupt file '%s' at line %d [entries] !", (*pwm)->file, count);
      r = PWM_FILE_CORRUPT;
      break;
    }
    user = line_fields[0];
    salt_str = line_fields[1];
    hash_str = line_fields[2];
    if (! pwm_decode_hex_string(salt_str, salt, sizeof(salt_t))) {
      pwm_error("Corrupt file '%s' at line %d [salt] !", (*pwm)->file, count);
      r = PWM_FILE_CORRUPT;
      break;
    }
    if (! pwm_decode_hex_string(hash_str, hash, sizeof(hash_t))) {
      pwm_error("Corrupt file '%s' at line %d [hash] !", (*pwm)->file, count);
      r = PWM_FILE_CORRUPT;
      break;
    }
    node = create_node(user, salt, hash);
    if (count == 1) {
      if (strcmp(node->user, PWM_ADMIN_USER) != 0) {
        r = PWM_FILE_CORRUPT;
        pwm_error("Corrupt file '%s': first user must be '%s'!", (*pwm)->file, PWM_ADMIN_USER);
        free(node);
        break;
      }
      hash_t vhash;
      pwm_hash_password(node->user, node->salt, password, vhash);
      if (memcmp(node->hash, vhash, sizeof(hash_t)) != 0) {
        r = PWM_PASSWORD_MISMATCH;
        pwm_error("Password mismatch for admin user!");
        free(node);
        break;
      }
      (*pwm)->entries = node;
    } else {
      curr->next = node;
    }  
    curr = node;
  }

  if (r == PWM_OK && count == 0) {
    r = PWM_FILE_CORRUPT;
    pwm_error("Corrupt file '%s': empty password database!", (*pwm)->file);
  }

decrypt_fail:
  if (parse_f && parse_f != f) {
    fclose(parse_f);
  }
  if (decrypted_plaintext) {
    free(decrypted_plaintext);
  }
  fclose(f);

  if (r != PWM_OK) {
    pwm_free(*pwm); 
    *pwm = NULL;
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
           && (r = pwm_hash_password(node -> user, salt, password, hash)) == PWM_OK) {
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
      pwm_node_t* to_delete = node -> next; // Guarda o nó a apagar
      node -> next = to_delete -> next;
      free(to_delete);
    } 
  }
  return r;
}

pwm_res_t pwm_save(PWM pwm) {
  char* plaintext = NULL;
  size_t plaintext_len = 0;
  FILE* mem = open_memstream(&plaintext, &plaintext_len);
  if (!mem) {
    pwm_error("Failed to allocate memory stream!");
    return PWM_MEMORY_ALLOCATION_ERROR;
  }
  
  pwm_node_t* node = pwm -> entries;
  while (node != NULL) {
    fprintf(mem, "%s:", node -> user);
    pwm_print_hex_string(mem, node -> salt, sizeof(salt_t));
    fputc(':', mem);
    pwm_print_hex_string(mem, node -> hash, sizeof(hash_t));
    fputc('\n', mem);
    node = node -> next;
  }
  fclose(mem);

  FILE* f = fopen(pwm -> file, "w");
  if (!f) {
    perror(pwm -> file);
    pwm_error("Could not open file '%s' for writing!", pwm -> file);
    free(plaintext);
    return PWM_IO_ERROR;
  }

  unsigned char kdf_salt[KDF_SALT_LEN];
  unsigned char iv[GCM_IV_LEN];
  if (1 != RAND_bytes(kdf_salt, KDF_SALT_LEN) || 1 != RAND_bytes(iv, GCM_IV_LEN)) {
    pwm_error("Failed to generate random bytes for crypto!");
    fclose(f);
    free(plaintext);
    return PWM_OS_ERROR;
  }

  unsigned char key[32];
  if (1 != PKCS5_PBKDF2_HMAC(pwm->admin_password, strlen(pwm->admin_password),
                             kdf_salt, KDF_SALT_LEN, KDF_ITERATIONS,
                             EVP_sha256(), 32, key)) {
    pwm_error("PBKDF2 key derivation failed!");
    fclose(f);
    free(plaintext);
    return PWM_OS_ERROR;
  }

  unsigned char* ciphertext = malloc(plaintext_len + 16);
  unsigned char tag[GCM_TAG_LEN];
  int ciphertext_len = aes_gcm_encrypt((unsigned char*)plaintext, plaintext_len,
                                       key, iv, ciphertext, tag);
  
  memset(key, 0, sizeof(key));

  if (ciphertext_len < 0) {
    pwm_error("Encryption failed!");
    free(ciphertext);
    free(plaintext);
    fclose(f);
    return PWM_OS_ERROR;
  }

  fwrite(AEAD_MAGIC, 1, AEAD_MAGIC_LEN, f);
  fwrite(kdf_salt, 1, KDF_SALT_LEN, f);
  fwrite(iv, 1, GCM_IV_LEN, f);
  fwrite(tag, 1, GCM_TAG_LEN, f);
  fwrite(ciphertext, 1, ciphertext_len, f);

  free(ciphertext);
  free(plaintext);
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
  if ((r = pwm_hash_password(node -> user, node -> salt, password, hash)) == PWM_OK) {
    if (memcmp(node -> hash, hash, sizeof(hash_t)) != 0) {
      pwm_error("Password mismatch for user '%s'!", user);
      r = PWM_PASSWORD_MISMATCH; 
    }
  }  
  return r;
}

