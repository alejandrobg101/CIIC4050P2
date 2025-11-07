#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "functions.h"

int main() {
  // TODO(HectorRivera1) 1: Call the function that creates the shared memory segment.
  MemoryCreate();
  // fprintf(stderr, "Shared memory segment created. Main PID: %d\n", shared_memory[0]);

  // TODO(HectorRivera1) 2: Configure the SIGUSR2 signal to increment the planes on the runway
  // by 5.
  struct sigaction sa2;
  sa2.sa_handler = SigHandler2;
  sigaction(SIGUSR2, &sa2, NULL);
  // fprintf(stderr, "SIGUSR2 handler configured.\n");

  // TODO(HectorRivera1) 3: Launch the 'radio' executable and, once launched, store its PID in
  // the second position of the shared memory block.
  pid_t radio_pid = fork();
  if (radio_pid == 0) {
    // Store the radio PID in shared memory
    shared_memory[1] = getpid();
    // Child process: Execute the 'radio' program
    if(execl("./radio", "radio", SH_MEMORY_NAME, NULL) == -1) {
      perror("Failed to launch radio process");
      exit(EXIT_FAILURE);
      return 1;
    }
  }
  // fprintf(stderr, "Radio process launched with PID: %d\n", radio_pid);
 
  // pid_t ground_control_pid = fork();
  // if (ground_control_pid == 0) {
  //   // Store the ground control PID in shared memory
  //   shared_memory[2] = getpid();
  //   // Child process: Execute the 'ground_control' program
  //   if(execl("./ground_control", "ground_control", NULL) == -1) {
  //     perror("Failed to launch ground_control process");
  //     exit(EXIT_FAILURE);
  //     return 1;
  //   }
  // }
  // fprintf(stderr, "Ground Control process launched with PID: %d\n", ground_control_pid);

  // TODO(HectorRivera1) 4: Launch 5 threads which will be the controllers; each thread will
  // execute the TakeOffsFunction().
  pthread_t controllers[5];
  for (int i = 0; i < 5; i++) {
    // TODO(HectorRivera1) 5: Syncronize the controllers using mutexes to avoid race conditions
    // when accessing shared variables. (Happens inside TakeOffsFunction)
    int create = pthread_create(&controllers[i], NULL, TakeOffsFunction, NULL);
    if (create != 0) {
      perror("Failed to create controller thread");
      exit(1);
    }
  }
  // fprintf(stderr, "All controller threads launched.\n");

  // TODO(HectorRivera1) 6: Send SIGUSR1 once takeoffs reach maximum (20).
  // (Happens inside TakeOffsFunction)
  
  // Wait for all controller threads to finish
  for (int i = 0; i < 5; i++) {
    pthread_join(controllers[i], NULL);
  }
  // fprintf(stderr, "All controller threads have finished.\n");

  // Cleanup: Unlink shared memory
  shm_unlink(SH_MEMORY_NAME);
  munmap(shared_memory, SH_MEMORY_SIZE);
  close(shm_fd);

  // Destroy mutexes
  pthread_mutex_destroy(&state_lock);
  pthread_mutex_destroy(&runway1_lock);
  pthread_mutex_destroy(&runway2_lock);

  // fprintf(stderr, "Program completed successfully.\n");
  return 0;
}
