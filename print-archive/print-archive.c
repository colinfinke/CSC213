#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_contents(uint8_t* data, size_t size);

int main(int argc, char** argv) {
  // Make sure we have a file input
  if (argc != 2) {
    fprintf(stderr, "Please specify an input filename.\n");
    exit(1);
  }

  // Try to open the file
  FILE* input = fopen(argv[1], "r");
  if (input == NULL) {
    perror("Unable to open input file");
    exit(1);
  }

  // Seek to the end of the file so we can get its size
  if (fseek(input, 0, SEEK_END) != 0) {
    perror("Unable to seek to end of file");
    exit(2);
  }

  // Get the size of the file
  size_t size = ftell(input);

  // Seek back to the beginning of the file
  if (fseek(input, 0, SEEK_SET) != 0) {
    perror("Unable to seek to beginning of file");
    exit(2);
  }

  // Allocate a buffer to hold the file contents. We know the size in bytes, so
  // there's no need to multiply to get the size we pass to malloc in this case.
  uint8_t* data = malloc(size);

  // Read the file contents
  if (fread(data, 1, size, input) != size) {
    fprintf(stderr, "Failed to read entire file\n");
    exit(2);
  }

  // Make sure the file starts with the .ar file signature
  if (memcmp(data, "!<arch>\n", 8) != 0) {
    fprintf(stderr, "Input file is not in valid .ar format\n");
    exit(1);
  }

  // Call the code to print the archive contents
  print_contents(data, size);

  // Clean up
  free(data);
  fclose(input);

  return 0;
}

/**
 * This function should print the name of each file in the archive followed by its contents.
 *
 * \param data This is a pointer to the first byte in the file.
 * \param size This is the number of bytes in the file.
 */
void print_contents(uint8_t* data, size_t size) {
  //skipping the !<arch> at the beginning of the input
  for(int i = 0; i<8; i++){
    data++;
  }

  //making an array to keep track of file size char, int for the file size, 
  //and uint8_t pointer to itrate through
  char fileSizeArr[10];
  int fileSize;
  uint8_t* pos = data;

  //iterates through the file throughout the size of the data
  while((int) pos <= size+(int) data - 10){
    //prints all of the file name chars that aren't space, /, or \n
    for(int i=0; i<16; i++){
      if(*pos != ' ' && *pos != '/' && *pos != '\n') printf("%c", *pos);
      pos++;
    }

    //moves pos to the file size
    for(int i=0; i<32; i++){
      pos++;
    }

    //makes an char array of the chars for the file size
    for(int i=0; i< 10; i++){
      fileSizeArr[i] = *pos;
      pos++;
    }

    //gets the file size (int) from the array of file size chars
    fileSize = strtod(fileSizeArr, NULL);

    //skips the rest of the file header
    while(*pos != '`'){
      pos++;
    }
    pos++;

    //prints to next for formatting
    printf("\n");

    //loops fileSize+1 times and prints all characters that aren't a new line
    for(int i=0; i<=fileSize; i++){
      if(*pos != '\n') printf("%c", *pos);
      pos++;
    }
    printf("\n");
  }
}