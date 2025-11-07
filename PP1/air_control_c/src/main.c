#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "functions.h"

int main() {
  // TODO 1: Call the function that creates the shared memory segment.
  MemoryCreate();
  // TODO 2: Configure the SIGUSR2 signal to increment the planes on the runway
  // by 5.
  struct sigaction sa2;
  sa2.sa_handler = SigHandler2;
  sigemptyset(&sa2.sa_mask);
  sa2.sa_flags = 0;
  sigaction(SIGUSR2, &sa2, NULL);

  // TODO 3: Launch the 'radio' executable and, once launched, store its PID in
  // the second position of the shared memory block.
  pid_t radio_pid = fork();
  if (radio_pid == 0) {
    // Child process: Execute the 'radio' program
    execl("./radio", "radio", SH_MEMORY_NAME, NULL);
    perror("Failed to exec radio");
    exit(1);
  }
  // Store the radio PID in shared memory
  shared_memory[1] = radio_pid;

  // TODO 4: Launch 5 threads which will be the controllers; each thread will
  // execute the TakeOffsFunction().
  pthread_t controllers[5];
  for (int i = 0; i < 5; i++) {
    // TODO 5: Syncronize the controllers using mutexes to avoid race conditions
    // when accessing shared variables. (Happens inside TakeOffsFunction)
    pthread_create(&controllers[i], NULL, TakeOffsFunction, NULL);
  }

  // TODO 6: Send SIGUSR1 once takeoffs reach maximum (20).
  // (Happens inside TakeOffsFunction)
  
  // Wait for all controller threads to finish
  for (int i = 0; i < 5; i++) {
    pthread_join(controllers[i], NULL);
  }
  shm_unlink(SH_MEMORY_NAME);
  return 0;
}
