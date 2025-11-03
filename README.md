This project aims to implement a basic aircraft takeoff control system using shared memory, signals, and threads in C and Python. Students will coordinate three processes (air_control, radio, and ground_control) that interact through shared PIDs, signals (SIGUSR1, SIGUSR2, SIGTERM), and resource synchronization (takeoff runways).


IMPORTANT: The radio executable is already provided in the basic project structure. The source code folder is attached if you want to find out what the program does. However, the executable to be used for grading is the original radio executable.
Note 1: This project is designed to be completed in groups of two.
Note 2: For proper program execution, you must save the PID for each program in shared memory. The order of the PIDs recorded in shared memory is as follows:

air_control_py PID

radio PID

ground_control_py PID

 
Part 1: C Implementation:

air_control_c:
In this first part, you must implement the air_control program, which simulates a basic aircraft takeoff control system using shared memory, signals, and threads in C. The objective is to replicate the interprocess coordination and thread synchronization logic, which will later be extended to the Python implementation. The main program will be responsible for creating shared memory, handling signals indicating the arrival of new aircraft, creating takeoff controller threads, and launching the radio process.
General Instructions

Create shared memory: The program must create a shared memory block large enough to store three integer values. The PIDs of the processes involved in the simulation will be stored in this memory.
Configure the signal handler: Register a handler for SIGUSR2 that increments the number of available planes for takeoff by 5.
 Launch the radio process: The air_control program must create a child process using fork(), and in that process, execute the radio binary using execl(). When launched, it must send the name of the shared memory (SH_MEMORY_NAME) as a parameter.
Create the controller threads: Five threads must be created, each executing the TakeOffsFunction function, which will be responsible for managing aircraft takeoffs.
Synchronize access to shared variables: Access to global variables and takeoff runways must be controlled using mutexes to avoid race conditions between threads.
Once the total number of takeoffs (TOTAL_TAKEOFFS) is reached, the air_control process must send the SIGTERM signal to the radio process and then release the shared memory.
Shared global variables (all are integers of type int):

planes: number of planes waiting to take off
takeoffs: count of takeoffs performed to account for blocks of 5 takeoffs and send the SIGUSR1 signal.
total_takeoffs: total takeoffs performed
Mutexes to be used:

state_lock: to protect and synchronize access to the shared variables indicated above, avoiding race conditions.
runway1_lock and runway2_lock: mutexes that simulate exclusive use of runways; only one thread can use a runway at a time.
Specifying tasks to be performed

Configure shared memory:
a.    Create the memory.
b.    Map the memory block.
c.    Save the PID of the air_control process in the first position of the memory block.
Signal Configuration:
a.    Configure the SigHandler2 handler for the SIGUSR2 signal.
b.    This handler must increment the planes variable by 5 units each time the signal is received.
Launching the radio process:
a.    Using fork(), create a child process.
b.    In the child process, save its PID in the second position of the memory block and run the radio program.
Creating Threads:
a.    Create 5 threads that execute the TakeOffsFunction function.
b.    Each thread will simulate an independent track controller.
Implementing TakeOffsFunction:
a.    While total_takeoffs < TOTAL_TAKEOFFS:
     - Attempt to take one of the two tracks using pthread_mutex_trylock().
     - If unsuccessful, sleep with usleep(1000) and retry.
b.    Once a runway is obtained, enter the mutex-protected critical section (this is if planes are available):
     - Decrement planes by 1.
     - Increase takeoffs and total_takeoffs by 1.
     - If takeoffs equals 5, send SIGUSR1 signal to the radio process and reset takeoffs = 0.
c.    Release the main mutex state_lock.
d.    Sleep sleep(1) to simulate the takeoff time.
e.    Release the runway (pthread_mutex_unlock() of runway1_lock or runway2_lock as appropriate).
f.    Upon completion, when total_takeoffs >= TOTAL_TAKEOFFS, send SIGTERM to the radio process, close shared memory, and return.
Release resources: Once all threads have finished, call shm_unlink(SH_MEMORY_NAME) to remove the shared memory.
ground_control:
The ground_control process simulates air traffic management on the ground. Its main functions are:

Register its PID in shared memory.
 Monitor the number of aircraft available for takeoff.
Notify the radio process when aircraft are added.
Receive radio signals and react to events such as takeoff blocks (SIGUSR1) or termination (SIGTERM).
Run a periodic timer to monitor traffic and control aircraft flow to the runways.
Specification of tasks to be performed

Open and map shared memory and save ground_control PID.
Signal handlers:
a.    SIGTERM: close descriptor, print message, exit.
b.    SIGUSR1: increment takeoffs by 5.
Configure setitimer to run Traffic every 500 ms.
Traffic:
a.    Calculate the number of planes waiting.
b.    If planes in line >= 10, print "RUNWAY OVERLOADED".
c.    If planes < PLANES_LIMIT, increase planes by 5 and send SIGUSR2 to the radio.
Radio communication:
a.    Send SIGUSR2 signals when new aircraft are added.
b.    Receive SIGUSR1 to update takeoffs.
6.    Upon receiving SIGTERM, the memory descriptor must be closed.
Example logs, part 1:
A screen shot of a computer

AI-generated content may be incorrect.A screen shot of a computer

AI-generated content may be incorrect.

Please note: You should not make any changes to the radio program; the radio program will handle all prints for test validation.


Part 2: Implementing Air Control in Python:
In this part, you will implement the air_control_py program. This program simulates basic aircraft takeoff control using shared memory, signals, and threads in Python, replicating the approach from the first part of the programming project. Similarly, this program sends the SIGUSR1 and SIGTERM signals to the radio program.

General instructions:

Complete the function to manage takeoffs in the threads.
Correctly configure the signals to update the available aircraft list.
Create a shared memory segment to store the PIDs of the involved processes.
Launch the radio program.
Synchronize access to shared variables using locks.
Global shared variables (all are integers of type int):

planes: number of aircraft waiting to take off
takeoffs: count of takeoffs performed to account for blocks of 5 takeoffs and send the SIGUSR1 signal.
total_takeoffs: total takeoffs performed
Locks (mutex) to be used:

state_lock: to protect and synchronize access to the shared variables indicated above, avoiding race conditions.
runway1_lock and runway2_lock: mutexes that simulate exclusive use of takeoff runways; only one thread can use a runway at a time.
Specification of tasks to be performed:

Keep in mind that you should only use the libraries that are part of the base program.
You must configure the SIGUSR2 signal handler. This handler will increment the planner variable by 5, indicating that 5 aircraft are on the runway for takeoff.
You must create a shared memory segment where each programs' PID will be recorded. This memory is a vector of size 3 ints.
Once the shared memory is created, the PID of the air_control_py program will be saved in the first position of the memory block.
You must launch the radio executable (already included in the base program) and send the name of the shared memory to be used as a parameter. Once the radio program is launched, the air_control_py program must save the radio PID in shared memory, which is the second PID registered in the shared memory.
You must launch 5 threads, which will be the system controllers. They will execute the TakeOffFunction function, responsible for authorizing takeoffs.
You must develop the TakeOffFunction function so that the threads consume planes from the planes variable and update the takeoffs and total_takeoffs counters safely and without conflicts. Keep in mind that:
a.    There are only 2 runways available, which is why they must be protected with mutexes. Each thread will be able to use one runway, and upon completion, it will release the runway in question. You should learn how threading.Lock() works.
b.    The increment of the variables planes (aircraft on hold), takeoffs (temporary takeoff count), and total_takeoffs (global takeoff count) must be protected with an additional mutex; at this point, planes would decrease by 1, takeoffs and total_takeoffs would increase by 1.
c.    Once takeoffs equals 5, the SIGUSR1 signal must be sent to the radio program. The takeoffs variable must be reset to 0, and the takeoff time will be simulated with a 1 second sleep().
d.    The thread must validate whether the takeoffs corresponding to TOTAL_TAKEOFFS have already been performed. If so, it must send the SIGTERM signal to the radio program, which then terminates the ground_control program.
Finally, you must execute a memory.close().
IMPORTANT: This program should also not have any prints. It is recommended to launch the air_control_py and ground_control programs manually and then run the tests, verifying that there are no prints. The logs for this program when running the tests will be in the air_control_py.log file.

