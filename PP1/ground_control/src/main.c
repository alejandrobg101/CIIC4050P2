
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>

void Traffic(int signum);
void MemoryOpen();
void SigHandlerTerm(int signum);
void SigHandler1(int signum);

#define PLANES_LIMIT 20
int planes = 0;
int takeoffs = 0;
int traffic = 0;

#define SH_MEMORY_NAME "/air_&_ground_control_shm"
#define SH_MEMORY_SIZE sizeof(pid_t) * 3
pid_t* shared_memory;
int shm_fd;

void Traffic(int signum) {
  // TODO(HectorRivera1):
  // Calculate the number of waiting planes.
  // Check if there are 10 or more waiting planes to send a signal and increment
  // planes. Ensure signals are sent and planes are incremented only if the
  // total number of planes has not been exceeded.
  traffic = planes - takeoffs;
  if (traffic >= 10) {
    // fprintf(stderr, "RUNWAY OVERLOADED\n");
  }

  if (planes < PLANES_LIMIT) {
    // fprintf(stderr, "Traffic: Adding 5 planes to the runway.\n");
    planes += 5;
    kill(shared_memory[1], SIGUSR2);
  }
}

int main(int argc, char* argv[]) {
  // TODO(HectorRivera1):
  // 1. Open the shared memory block and store this process PID in position 2
  //    of the memory block.
  // 3. Configure SIGTERM and SIGUSR1 handlers
  //    - The SIGTERM handler should: close the shared memory, print
  //      "finalization of operations..." and terminate the program.
  //    - The SIGUSR1 handler should: increase the number of takeoffs by 5.
  // 2. Configure the timer to execute the Traffic function.

  // TODO(HectorRivera1) 1: Open shared memory and store PID
  // fprintf(stderr, "Opening shared memory segment...\n");
  MemoryOpen();
  // fprintf(stderr, "Shared memory segment opened."+
  // " Main PID: %d\n", shared_memory[2]);

  // TODO(HectorRivera1) 2: Configure signal handlers
  // SIGTERM
  struct sigaction sa_term;
  sa_term.sa_handler = SigHandlerTerm;
  sigaction(SIGTERM, &sa_term, NULL);
  // fprintf(stderr, "SIGTERM handler configured.\n");
  // SIGUSR1
  struct sigaction sa_usr1;
  sa_usr1.sa_handler = SigHandler1;
  sigaction(SIGUSR1, &sa_usr1, NULL);
  // fprintf(stderr, "SIGUSR1 handler configured.\n");

  // TODO(HectorRivera1) 3: Configure timer to execute Traffic function
  signal(SIGALRM, Traffic);
  struct itimerval timer;
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 500000;  // First timer after 500ms
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 500000;  // Subsequent timers every 500ms

  if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
    perror("Error setting timer");
    exit(EXIT_FAILURE);
  }
  // fprintf(stderr, "Timer configured to trigger "+
  // "Traffic function every 500ms.\n");

  // Keep the program running to handle signals
  // fprintf(stderr, "Entering main loop to handle signals...\n");
  while (1) {
    pause();
  }

  // fprintf(stderr, "Finalization of operations...\n");
  return 0;
}

void MemoryOpen() {
  // Open shared memory
  shm_fd = shm_open(SH_MEMORY_NAME, O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open");
    exit(EXIT_FAILURE);
  }

  // Map shared memory
  shared_memory = mmap(0, SH_MEMORY_SIZE,
     PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shared_memory == MAP_FAILED) {
    perror("mmap failed");
    exit(EXIT_FAILURE);
  }

  // Store PID in shared memory
  shared_memory[2] = getpid();
}

void SigHandlerTerm(int signum) {
  // Close shared memory
  munmap(shared_memory, SH_MEMORY_SIZE);
  close(shm_fd);
  printf("finalization of operations...\n");
  exit(EXIT_SUCCESS);
}

void SigHandler1(int signum) {
  takeoffs += 5;
  // fprintf(stderr, "Signal SIGUSR1 received:"+
  // " Takeoffs increased by 5 to %d\n", takeoffs);
}
