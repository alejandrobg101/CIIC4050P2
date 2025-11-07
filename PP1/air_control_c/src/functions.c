#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "functions.h"

// Global variables
int planes = 0;
int takeoffs = 0;
int total_takeoffs = 0;
#define TOTAL_TAKEOFFS 20

// Mutexes
pthread_mutex_t state_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t runway1_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t runway2_lock = PTHREAD_MUTEX_INITIALIZER;

// Shared memory pointer
#define SH_MEMORY_NAME "/air_control_shm"
#define SH_MEMORY_SIZE sizeof(pid_t) * 3
pid_t* shared_memory;
int shm_fd;

// TODO 1: Function to create shared memory segment
void MemoryCreate() {
  // Create shared memory segment
  shm_fd = shm_open(SH_MEMORY_NAME, O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open failed");
    exit(EXIT_FAILURE);
  }

  // Configure the size of the shared memory segment
  if (ftruncate(shm_fd, SH_MEMORY_SIZE) == -1) {
    perror("ftruncate failed");
    exit(EXIT_FAILURE);
  }

  // Map the shared memory segment to the process address space
  shared_memory = mmap(0, SH_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

  if (shared_memory == MAP_FAILED) {
    perror("mmap failed");
    exit(EXIT_FAILURE);
  }

  // Store the main process PID in the first position
  shared_memory[0] = getpid();
}

// TODO 2: Signal handler for SIGUSR2
void SigHandler2(int signal) {
    // Increment planes by 5
    planes += 5;
    fprintf(stderr, "Signal SIGUSR2 received: Incremented planes by 5. Total planes: %d\n", planes);
}

// TODO 4: Function executed by controller threads
void* TakeOffsFunction() {
  // Variable
  int current_runway;

  // Loop until total takeoffs reach the limit
  while (total_takeoffs < TOTAL_TAKEOFFS) {
    // Try to acquire a runway
    if (pthread_mutex_trylock(&runway1_lock) == 0) {
      current_runway = 1;
    } else if (pthread_mutex_trylock(&runway2_lock) == 0) {
      current_runway = 2;
    } else {
      // Both runways are busy, wait and retry
      usleep(1000);
      continue;
    }

    // TODO 5: Synchronize access to shared variables using mutexes
    // Runway found - perform takeoff
    pthread_mutex_lock(&state_lock);
    planes--;
    takeoffs++;
    total_takeoffs++;
    fprintf(stderr, "Takeoff from runway %d. Remaining planes: %d\n", current_runway, planes);

    // Signal if 5 takeoffs have occurred
    if (takeoffs == 5) {
      kill(getpid(), SIGUSR1);
      takeoffs = 0;
    }

    pthread_mutex_unlock(&state_lock);
    sleep(1);

    // Release the locked runway
    if (current_runway == 1) {
      pthread_mutex_unlock(&runway1_lock);
    } else {
      pthread_mutex_unlock(&runway2_lock);
    }
  }

  // TODO 6: Send SIGUSR1 once takeoffs reach maximum (20).
  // Terminate the process after completing max takeoffs
  kill(getpid(), SIGTERM);
  return NULL;
}