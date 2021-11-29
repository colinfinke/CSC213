#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>

#define PAGE_SIZE 0x1000
#define ROUND_UP(x,y) ((x) % (y) == 0 ? (x) : (x) + ((y) - (x) % (y)))

//lock to prevent threads from mixing up values
pthread_mutex_t lock;

//arguments to pass to threads
typedef struct thread_arg {
  int num_threads;
  off_t file_size;
  char* file_data;
  int threadID;
} thread_arg_t;

/// The number of times we've seen each letter in the input, initially zero
size_t letter_counts[26] = {0};

//function for threads
//splits the file_data into num_threads even (or almost) pieces and counts how many times a char appears
void* count_letter_fn(void* p){
  //getting arguments from p
  thread_arg_t* arg = (thread_arg_t*)p;
  int num_threads = arg->num_threads;
  size_t file_size = arg->file_size;
  char* file_data = arg->file_data;
  int threadID = arg->threadID;

  //calculates the starting position and end position based on number of threads
  size_t start_pos = (file_size+num_threads /*make sure it rounds up*/) / num_threads * threadIdx.x; //divides file into pieces
  size_t end_pos = start_pos + (file_size+num_threads) / num_threads;

  //given code: counts character appearences
  for(size_t i = start_pos; i< end_pos; i++) {
    char c = file_data[i];
    if(c >= 'a' && c <= 'z') {
      pthread_mutex_lock(&lock); //locks thread
      letter_counts[c - 'a']++;
      pthread_mutex_unlock(&lock); //unlocks thread
    } else if(c >= 'A' && c <= 'Z') {
      pthread_mutex_lock(&lock); //locks threads
      letter_counts[c - 'A']++;
      pthread_mutex_unlock(&lock); //unlocks threads
    }
  }

  return NULL;
}

/**
 * This function should divide up the file_data between the specified number of
 * threads, then have each thread count the number of occurrences of each letter
 * in the input data. Counts should be written to the letter_counts array. Make
 * sure you count only the 26 different letters, treating upper- and lower-case
 * letters as the same. Skip all other characters.
 *
 * \param num_threads   The number of threads your program must use to count
 *                      letters. Divide work evenly between threads
 * \param file_data     A pointer to the beginning of the file data array
 * \param file_size     The number of bytes in the file_data array
 */
void count_letters(int num_threads, char* file_data, off_t file_size) {
  //sets up arrays to keep track of threads and args
  pthread_t count_letter_th[num_threads];
  thread_arg_t args[num_threads];

  //initializes lock
  if(pthread_mutex_init(&lock, NULL) != 0){
    perror("Failed pthread_mutex_init for lock\n");
    exit(-1);
  }

  //sets up args and then creates threads
  for(int i=0; i<num_threads; i++){
    args[i].num_threads = num_threads;
    args[i].file_data = file_data;
    args[i].file_size = file_size;
    args[i].threadID = i;
    if (pthread_create(&count_letter_th[i], NULL, count_letter_fn, &args[i])) {
        perror("pthread_create failed");
        exit(EXIT_FAILURE);
    }
  }

  //joins all of the threads
  for(int i=0; i<num_threads; i++){
    if (pthread_join(count_letter_th[i], NULL)) {
        perror("pthread_join failed");
        exit(EXIT_FAILURE);
    }
  }
}

/**
 * Show instructions on how to run the program.
 * \param program_name  The name of the command to print in the usage info
 */
void show_usage(char* program_name) {
  fprintf(stderr, "Usage: %s <N> <input file>\n", program_name);
  fprintf(stderr, "    where <N> is the number of threads (1, 2, 4, or 8)\n");
  fprintf(stderr, "    and <input file> is a path to an input text file.\n");
}

int main(int argc, char** argv) {
  // Check parameter count
  if(argc != 3) {
    show_usage(argv[0]);
    exit(1);
  }

  // Read thread count
  int num_threads = atoi(argv[1]);
  if(num_threads != 1 && num_threads != 2 && num_threads != 4 && num_threads != 8) {
    fprintf(stderr, "Invalid number of threads: %s\n", argv[1]);
    show_usage(argv[0]);
    exit(1);
  }

  // Open the input file
  int fd = open(argv[2], O_RDONLY);
  if(fd == -1) {
    fprintf(stderr, "Unable to open input file: %s\n", argv[2]);
    show_usage(argv[0]);
    exit(1);
  }

  // Get the file size
  off_t file_size = lseek(fd, 0, SEEK_END);
  if(file_size == -1) {
    fprintf(stderr, "Unable to seek to end of file\n");
    exit(2);
  }

  // Seek back to the start of the file
  if(lseek(fd, 0, SEEK_SET)) {
    fprintf(stderr, "Unable to seek to the beginning of the file\n");
    exit(2);
  }

  // Load the file with mmap
  char* file_data = mmap(NULL, ROUND_UP(file_size, PAGE_SIZE), PROT_READ, MAP_PRIVATE, fd, 0);
  if(file_data == MAP_FAILED) {
    fprintf(stderr, "Failed to map file\n");
    exit(2);
  }

  // Call the function to count letter frequencies
  count_letters(num_threads, file_data, file_size);

  // Print the letter counts
  for(int i=0; i<26; i++) {
    printf("%c: %lu\n", 'a' + i, letter_counts[i]);
  }

  return 0;
}
