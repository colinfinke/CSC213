#define _GNU_SOURCE
#include <openssl/md5.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_USERNAME_LENGTH 64
#define PASSWORD_LENGTH 6

/************************* Part A *************************/

/**
 * Find a six character lower-case alphabetic password that hashes
 * to the given hash value. Complete this function for part A of the lab.
 *
 * \param input_hash  An array of MD5_DIGEST_LENGTH bytes that holds the hash of a password
 * \param output      A pointer to memory with space for a six character password + '\0'
 * \returns           0 if the password was cracked. -1 otherwise.
 */
int crack_single_password(uint8_t* input_hash, char* output) {
  // This variable holds the password we are trying
  // Start it one before "aaaaaa" because we iterate before we check it against the given hash
  char candidate_passwd[] = "aaaaa`";

  // Iterate through all possible passwords, starting with "aaaaaa" and ending with "zzzzzz"
  while (strcmp(candidate_passwd, "zzzzzz") != 0) {
    if (candidate_passwd[PASSWORD_LENGTH - 1] != 'z') {
      // Increment the rightmost digit
      candidate_passwd[PASSWORD_LENGTH - 1]++;
    } else {
      // We need to "roll over" the rightmost digit (and possibly other digits as well)
      candidate_passwd[PASSWORD_LENGTH - 1] = 'a';
      for (int i = PASSWORD_LENGTH - 2; i >= 0; i--) {
        if (candidate_passwd[i] != 'z') {
          candidate_passwd[i] = (char)(candidate_passwd[i] + 1);
          break;
        } else {
          candidate_passwd[i] = 'a';
        }
      }
    }

    // Take our candidate password and hash it using MD5
    uint8_t
        candidate_hash[MD5_DIGEST_LENGTH];  //< This will hold the hash of the candidate password
    MD5((unsigned char*)candidate_passwd, strlen(candidate_passwd),
        candidate_hash);  //< Do the hash

    // Now check if the hash of the candidate password matches the input hash
    if (memcmp(input_hash, candidate_hash, MD5_DIGEST_LENGTH) == 0) {
      // Match! Copy the password to the output and return 0 (success)
      strncpy(output, candidate_passwd, PASSWORD_LENGTH + 1);
      return 0;
    }
  }
  // We have checked every password, with no matches. Return failure.
  return -1;
}

/********************* Parts B & C ************************/

/**
 * This struct is the root of the data structure that will hold users and hashed passwords.
 * This could be any type of data structure you choose: list, array, tree, hash table, etc.
 * Implement this data structure for part B of the lab.
 */
typedef struct password_set {
  // Fields
  char* username;
  uint8_t* passwdHash;
  struct password_set* next;
  bool cracked;
} password_set_t;

/**
 * Initialize a password set.
 * Complete this implementation for part B of the lab.
 *
 * \param passwords  A pointer to allocated memory that will hold a password set
 */
void init_password_set(password_set_t* passwords) {
  // Initialize any fields for the password set structure
  passwords->username = NULL;
  passwords->passwdHash = NULL;
  passwords->next = NULL;
  passwords->cracked = false;
}

/**
 * Add a password to a password set
 * Complete this implementation for part B of the lab.
 *
 * \param passwords   A pointer to a password set initialized with the function above.
 * \param username    The name of the user being added. The memory that holds this string's
 *                    characters will be reused, so if you keep a copy you must duplicate the
 *                    string. I recommend calling strdup().
 * \param password_hash   An array of MD5_DIGEST_LENGTH bytes that holds the hash of this user's
 *                        password. The memory that holds this array will be reused, so you must
 *                        make a copy of this value if you retain it in your data structure.
 */
void add_password(password_set_t* passwords, char* username, uint8_t* password_hash) {
  // Add the provided user and password hash to the set of passwords

  // Reserve space for the given data
  password_set_t* newNode;
  if ((newNode = malloc(sizeof(password_set_t))) == NULL) {
    perror("malloc failed");
    exit(EXIT_FAILURE);
  }
  if ((newNode->username = malloc(MAX_USERNAME_LENGTH)) == NULL) {
    perror("malloc failed");
    exit(EXIT_FAILURE);
  }
  if ((newNode->passwdHash = malloc(MD5_DIGEST_LENGTH)) == NULL) {
    perror("malloc failed");
    exit(EXIT_FAILURE);
  }

  // Copy the given hashed password into the space we reserved for it
  for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
    newNode->passwdHash[i] = password_hash[i];
  }

  // Copy the given username into the space we reserved for it
  if ((newNode->username = strdup(username)) == NULL) {
    perror("strdup failed");
    exit(EXIT_FAILURE);
  }

  newNode->cracked = false;

  // Put our new node at the front of the passwords list, but behind a dummy node
  newNode->next = passwords->next;
  passwords->next = newNode;
}

// Structure to store thread arguments
typedef struct args {
  password_set_t* passwords;  //< Passwords list
  char* start;                //< First candidate password to check
  char* fin;                  //< Last candidate password to check
} args_t;

/**
 * Check a subset of all possible passwords against the list of hashed passwords
 *
 * \param p   A pointer (of type arg_t) which stores arguments
 * \return    The number of passwords cracked
 */
void* crack_password_thread(void* p) {
  bool allCracked = false;  //< Have all the passwords in the passwords list been cracked?
  args_t* args = (args_t*)p;

  char candidate_passwd[PASSWORD_LENGTH + 1];  //< This variable holds the password we are trying
  // Copy the first password to check into candidate_passwd
  for (int i = 0; i < PASSWORD_LENGTH + 1; i++) {
    candidate_passwd[i] = args->start[i];
  }
  candidate_passwd[PASSWORD_LENGTH] = '\0';

  // This variable will store the number of passwords we've cracked
  int* count;
  if ((count = malloc(sizeof(int))) == NULL) {
    perror("malloc failed");
    exit(EXIT_FAILURE);
  }
  *count = 0;

  // Advance to the next candidate password (as we do in part a), so long as we have not cracked
  // all the passwords in the list, the list is nonempty, and we haven't reached the last candidate
  while (!allCracked && args->passwords->next != NULL && strcmp(candidate_passwd, args->fin) != 0) {
    if (candidate_passwd[PASSWORD_LENGTH - 1] != 'z') {
      candidate_passwd[PASSWORD_LENGTH - 1]++;
    } else {
      candidate_passwd[PASSWORD_LENGTH - 1] = 'a';
      for (int i = PASSWORD_LENGTH - 2; i >= 0; i--) {
        if (candidate_passwd[i] != 'z') {
          candidate_passwd[i]++;
          break;
        } else {
          candidate_passwd[i] = 'a';
        }
      }
    }

    // Take our candidate password and hash it using MD5
    uint8_t
        candidate_hash[MD5_DIGEST_LENGTH];  //< This will hold the hash of the candidate password
    MD5((unsigned char*)candidate_passwd, strlen(candidate_passwd),
        candidate_hash);  //< Do the hash

    // Iterate through the passwords list, checking our candidate hash against each one
    password_set_t* cursor = args->passwords->next;
    allCracked = true;
    while (cursor != NULL) {
      // Now check if the hash of the candidate password matches the input hash
      if (!cursor->cracked && memcmp(cursor->passwdHash, candidate_hash, MD5_DIGEST_LENGTH) == 0) {
        // Match! Print the password and corresponding username and update cracked and count
        printf("%s %s\n", cursor->username, candidate_passwd);
        cursor->cracked = true;
        (*count)++;
      } else if (!cursor->cracked) {
        // If any passwords have not yet been cracked, set allCracked to false
        allCracked = false;
      }
      cursor = cursor->next;
    }
  }
  return count;
}

/**
 * Crack all of the passwords in a set of passwords. The function should print the username
 * and cracked password for each user listed in passwords, separated by a space character.
 * Complete this implementation for part B of the lab.
 *
 * \returns The number of passwords cracked in the list
 */
int crack_password_list(password_set_t* passwords) {
  int numThreads = 4;
  pthread_t threads[numThreads];
  args_t args[numThreads];

  args[0].passwords = passwords;
  args[1].passwords = passwords;
  args[2].passwords = passwords;
  args[3].passwords = passwords;

  // Set the first password candidate for each thread
  args[0].start = "aaaaa`";
  args[1].start = "gmzzzz";
  args[2].start = "mzzzzz";
  args[3].start = "tmzzzz";

  // Set the last password candidate for each thread
  args[0].fin = args[1].start;
  args[1].fin = args[2].start;
  args[2].fin = args[3].start;
  args[3].fin = "zzzzzz";

  // Create threads
  for (int i = 0; i < numThreads; i++) {
    if (pthread_create(&threads[i], NULL, crack_password_thread, &args[i])) {
      perror("pthread_create failed");
      exit(EXIT_FAILURE);
    }
  }

  int* ret[numThreads];  //< To store return values of threads

  // Join the threads
  for (int i = 0; i < numThreads; i++) {
    if (pthread_join(threads[i], (void**)&ret[i])) {
      perror("pthread_join failed");
      exit(EXIT_FAILURE);
    }
  }

  // Get the total number of passwords cracked
  int sum = 0;
  for (int i = 0; i < numThreads; i++) {
    sum += (*ret[i]);
  }
  return sum;
}

/******************** Provided Code ***********************/

/**
 * Convert a string representation of an MD5 hash to a sequence
 * of bytes. The input md5_string must be 32 characters long, and
 * the output buffer bytes must have room for MD5_DIGEST_LENGTH
 * bytes.
 *
 * \param md5_string  The md5 string representation
 * \param bytes       The destination buffer for the converted md5 hash
 * \returns           0 on success, -1 otherwise
 */
int md5_string_to_bytes(const char* md5_string, uint8_t* bytes) {
  // Check for a valid MD5 string
  if (strlen(md5_string) != 2 * MD5_DIGEST_LENGTH) return -1;

  // Start our "cursor" at the start of the string
  const char* pos = md5_string;

  // Loop until we've read enough bytes
  for (size_t i = 0; i < MD5_DIGEST_LENGTH; i++) {
    // Read one byte (two characters)
    int rc = sscanf(pos, "%2hhx", &bytes[i]);
    if (rc != 1) return -1;

    // Move the "cursor" to the next hexadecimal byte
    pos += 2;
  }

  return 0;
}

void print_usage(const char* exec_name) {
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  %s single <MD5 hash>\n", exec_name);
  fprintf(stderr, "  %s list <password file name>\n", exec_name);
}

int main(int argc, char** argv) {
  if (argc != 3) {
    print_usage(argv[0]);
    exit(1);
  }

  if (strcmp(argv[1], "single") == 0) {
    // The input MD5 hash is a string in hexadecimal. Convert it to bytes.
    uint8_t input_hash[MD5_DIGEST_LENGTH];
    if (md5_string_to_bytes(argv[2], input_hash)) {
      fprintf(stderr, "Input has value %s is not a valid MD5 hash.\n", argv[2]);
      exit(1);
    }

    // Now call the crack_single_password function
    char result[7];
    if (crack_single_password(input_hash, result)) {
      printf("No matching password found.\n");
    } else {
      printf("%s\n", result);
    }

  } else if (strcmp(argv[1], "list") == 0) {
    // Make and initialize a password set
    password_set_t passwords;
    init_password_set(&passwords);

    // Open the password file
    FILE* password_file = fopen(argv[2], "r");
    if (password_file == NULL) {
      perror("opening password file");
      exit(2);
    }

    int password_count = 0;

    // Read until we hit the end of the file
    while (!feof(password_file)) {
      // Make space to hold the username
      char username[MAX_USERNAME_LENGTH];

      // Make space to hold the MD5 string
      char md5_string[MD5_DIGEST_LENGTH * 2 + 1];

      // Make space to hold the MD5 bytes
      uint8_t password_hash[MD5_DIGEST_LENGTH];

      // Try to read. The space in the format string is required to eat the newline
      if (fscanf(password_file, "%s %s ", username, md5_string) != 2) {
        fprintf(stderr, "Error reading password file: malformed line\n");
        exit(2);
      }

      // Convert the MD5 string to MD5 bytes in our new node
      if (md5_string_to_bytes(md5_string, password_hash) != 0) {
        fprintf(stderr, "Error reading MD5\n");
        exit(2);
      }

      // Add the password to the password set
      add_password(&passwords, username, password_hash);
      password_count++;
    }

    // Now run the password list cracker
    int cracked = crack_password_list(&passwords);

    printf("Cracked %d of %d passwords.\n", cracked, password_count);

  } else {
    print_usage(argv[0]);
    exit(1);
  }

  return 0;
}
