#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "socket.h"
#include "ui.h"

#define MAXLENGTH 100

typedef struct port_set {
  // Fields
  unsigned short port_name;
  struct port_set* next;
} port_set_t;

void init_port_set(port_set_t* ports) {
  // Initialize any fields for the password set structure
  ports = NULL;
}

port_set_t* ports;

// Send a across a socket with a header that includes the message length.
int send_message(int fd, char* message) {
  // If the message is NULL, set errno to EINVAL and return an error
  if (message == NULL) {
    errno = EINVAL;
    return -1;
  }

  // First, send the length of the message in a size_t
  size_t len = strlen(message);
  if (write(fd, &len, sizeof(size_t)) != sizeof(size_t)) {
    // Writing failed, so return an error
    return -1;
  }

  // Now we can send the message. Loop until the entire message has been written.
  size_t bytes_written = 0;
  while (bytes_written < len) {
    // Try to write the entire remaining message
    ssize_t rc = write(fd, message + bytes_written, len - bytes_written);

    // Did the write fail? If so, return an error
    if (rc <= 0) return -1;

    // If there was no error, write returned the number of bytes written
    bytes_written += rc;
  }

  return 0;
}

// Receive a message from a socket and return the message string (which must be freed later)
char* receive_message(int fd) {
  // First try to read in the message length
  size_t len;
  if (read(fd, &len, sizeof(size_t)) != sizeof(size_t)) {
    // Reading failed. Return an error
    return NULL;
  }

  // Now make sure the message length is reasonable
  if (len > MAXLENGTH) {
    errno = EINVAL;
    return NULL;
  }

  // Allocate space for the message
  char* result = malloc(len+1);

  // Try to read the message. Loop until the entire message has been read.
  size_t bytes_read = 0;
  while (bytes_read < len) {
    // Try to read the entire remaining message
    ssize_t rc = read(fd, result + bytes_read, len - bytes_read);

    // Did the read fail? If so, return an error
    if (rc <= 0) {
      free(result);
      return NULL;
    }

    // Update the number of bytes read
    bytes_read += rc;
  }

  result[len] = '\0';
  return result;
}

// Keep the username in a global so we can access it from the callback
char* username;

void echo_message(char* message){
  port_set_t* cursor = ports;
  while(cursor != NULL){
    int rc = send_message(cursor->port_name, message);
    if (rc == -1) {
      perror("Failed to send message");
      exit(EXIT_FAILURE);
    }
    cursor = cursor->next;
    printf("MESSAGE SENT\n");
  }
}

// This function is run whenever the user hits enter after typing a message
void input_callback(const char* message) {
  if (strcmp(message, ":quit") == 0 || strcmp(message, ":q") == 0) {
    ui_exit();
  } else {
    char* temp_message = (char*) malloc(sizeof(message));
    strcpy(temp_message, message);
    
    ui_display(username, temp_message);

    echo_message(temp_message);

    free(temp_message);
  }
}

void* accept_thread(void* p){
  int server_socket_fd = *(int *) p;
  while(1){
    // Wait for a client to connect
  int client_socket_fd = server_socket_accept(server_socket_fd);
  if (client_socket_fd == -1) {
    perror("accept failed");
    exit(EXIT_FAILURE);
  }

  }

  return NULL;
}

void* recieve_thread(void* p){
  char* message;
  while(1){
    port_set_t* cursor = ports;
    while(cursor!=NULL){
    // Read a message from the client
    message = receive_message(cursor->port_name);
      if (message == NULL) {
        perror("Failed to read message from client");
        exit(EXIT_FAILURE);
      }
      cursor = cursor->next;
      ui_display("USER", message);
    }
  }

  return NULL;
}

//connect, accept, send, recieve
int main(int argc, char** argv) {
  // Make sure the arguments include a username
  if (argc != 2 && argc != 4) {
    fprintf(stderr, "Usage: %s <username> [<peer> <port number>]\n", argv[0]);
    exit(1);
  }

  // Save the username in a global
  username = argv[1];
  
  init_port_set(ports);


  // Set up a server socket to accept incoming connections
  // Open a server socket
  unsigned short port = 0;
  int server_socket_fd = server_socket_open(&port);
  if (server_socket_fd == -1) {
    perror("Server socket was not opened");
    exit(EXIT_FAILURE);
  }

    // Start listening for connections, with a maximum of one queued connection
  if (listen(server_socket_fd, 1)) {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %u\n", port);

  // Did the user specify a peer we should connect to?
  if (argc == 4) {
    // Unpack arguments
    char* peer_hostname = argv[2];
    unsigned short peer_port = atoi(argv[3]);

    // Connect to another peer in the chat network
    int socket_fd = socket_connect(peer_hostname, peer_port);
    if (socket_fd == -1) {
      perror("Failed to connect");
      exit(EXIT_FAILURE);
    }

    port_set_t* newNode = (port_set_t*) malloc(sizeof(port_set_t*));
    newNode->port_name = socket_fd;
    newNode->next = ports;
    ports = newNode;
  }
  


  pthread_t acceptThread;
  pthread_t recieveThread;

  if (pthread_create(&acceptThread, NULL, accept_thread, &server_socket_fd)) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  if (pthread_create(&recieveThread, NULL, recieve_thread, NULL)) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  // Set up the user interface. The input_callback function will be called
  // each time the user hits enter to send a message.
  ui_init(input_callback);

  char portChar[5];
  snprintf(portChar, 6, "%d\n", (int) port);
  
  // Once the UI is running, you can use it to display log messages
  ui_display("Port", portChar);

  // Run the UI loop. This function only returns once we call ui_stop() somewhere in the program.
  ui_run();

  close(server_socket_fd);

  return 0;
}
