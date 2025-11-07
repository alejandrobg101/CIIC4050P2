#ifndef AIR_CONTROL_C_INCLUDE_FUNCTIONS_H_
#define AIR_CONTROL_C_INCLUDE_FUNCTIONS_H_

#include <pthread.h>
#include <sys/types.h>
#include <signal.h>

#define TOTAL_TAKEOFFS 20
#define SH_MEMORY_NAME "/air_&_ground_control_shm"
#define SH_MEMORY_SIZE (sizeof(pid_t) * 3)

extern int planes;
extern int takeoffs;
extern int total_takeoffs;

extern pthread_mutex_t state_lock;
extern pthread_mutex_t runway1_lock;
extern pthread_mutex_t runway2_lock;

extern pid_t* shared_memory;
extern int shm_fd;

void MemoryCreate();
void* TakeOffsFunction();
void SigHandler2(int signal);

#endif  // AIR_CONTROL_C_INCLUDE_FUNCTIONS_H_