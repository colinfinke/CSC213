#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


void alarm_handler(int signal, siginfo_t* info, void* ctx);

// Implement signal handling code
__attribute__((constructor)) void init() {
  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_sigaction = alarm_handler;
  sa.sa_flags = SA_SIGINFO;

  if(sigaction(SIGSEGV, &sa, NULL) != 0){
    perror("sigaction failed");
    exit(2);
  }
}


void alarm_handler(int signal, siginfo_t* info, void* ctx){
  //array of possible error messages
  char* messages[8] =
      {"Oh no, I'm so sorry you messed up! There's a SEGFAULT! Keep trying, you got this!!",
      "Dang that sucks. You got a SEGFAULT my friend. Keep your head up. Maybe we'll get ice cream!",
      "Hey, you have a SEGFAULT, don't feel sad, it happens to all coders!",
      "Oof! SEGFAULT! That's not fun! This is enocuraging you! Try again!",
      "Hey, you know life is hard sometimes but so is a SEGFAULT. You're not alone in these troubling times!",
      "Uh oh my friend. SEGFAULTs happen to the best of us. Just more to you than me I guess.",
      "Oh dear!! You got a SEGFAULT, do better next time, We expect more from you!",
      "SEGFAULT! Maybe you should take a break, get some snacks, and try again!"};
  //picking random number and printing the corresponding message
  time_t t;
  srand((unsigned) &t);
  printf("%s\n", messages[rand()%8]);
  exit(0);
}
