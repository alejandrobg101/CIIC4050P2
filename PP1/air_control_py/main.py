import ctypes
import mmap
import os
import signal
import subprocess
import threading
import time

_libc = ctypes.CDLL(None, use_errno=True)
_libc.shm_open.argtypes = [ctypes.c_char_p, ctypes.c_int, ctypes.c_uint]
_libc.shm_open.restype = ctypes.c_int
_libc.ftruncate.argtypes = [ctypes.c_int, ctypes.c_long]
_libc.close.argtypes = [ctypes.c_int]
_libc.close.restype = ctypes.c_int
_libc.shm_unlink.argtypes = [ctypes.c_char_p]
_libc.shm_unlink.restype = ctypes.c_int

TOTAL_TAKEOFFS = 20
STRIPS = 5
SHM_NAME = b"/air_&_ground_control_shm"
SHM_SIZE = ctypes.sizeof(ctypes.c_int * 3)

planes = 0
takeoffs = 0
total_takeoffs = 0

state_lock = threading.Lock()
runway1_lock = threading.Lock()
runway2_lock = threading.Lock()

shm_fd = None
shm_mem = None
shm_view = None
radio_proc = None


def create_shared_memory():
    global shm_fd, shm_mem, shm_view
    shm_fd = _libc.shm_open(SHM_NAME, os.O_CREAT | os.O_RDWR, 0o666)
    if shm_fd == -1:
        errno_val = ctypes.get_errno()
        raise OSError(errno_val, f"shm_open failed: {os.strerror(errno_val)}")
    if _libc.ftruncate(shm_fd, SHM_SIZE) == -1:
        errno_val = ctypes.get_errno()
        raise OSError(errno_val, f"ftruncate failed: {os.strerror(errno_val)}")
    shm_mem = mmap.mmap(shm_fd, SHM_SIZE, mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE)
    # Create ctypes array from mmap buffer
    shm_view = (ctypes.c_int * 3).from_buffer(shm_mem)
    # Initialize all PIDs to 0
    shm_view[0] = 0
    shm_view[1] = 0
    shm_view[2] = 0
    # Store air_control_py PID in first position
    shm_view[0] = os.getpid()


def sigusr2_handler(signum, frame):
    global planes
    with state_lock:
        planes += 5


def takeoff_worker():
    global planes, takeoffs, total_takeoffs

    # Loop until total takeoffs reach the limit
    while total_takeoffs < TOTAL_TAKEOFFS:
        # Quick check if there are planes available (without lock for performance)
        if planes <= 0:
            # No planes available for takeoff, wait and retry
            time.sleep(0.001)
            continue

        # Try to acquire a runway (non-blocking)
        runway = None
        if runway1_lock.acquire(blocking=False):
            runway = 1
        elif runway2_lock.acquire(blocking=False):
            runway = 2
        else:
            # Both runways are busy, wait and retry
            time.sleep(0.001)
            continue

        # Runway found - perform takeoff (with state lock held)
        with state_lock:
            # Double-check planes are still available (with lock held)
            if planes <= 0:
                # Release runway before continuing
                if runway == 1:
                    runway1_lock.release()
                else:
                    runway2_lock.release()
                continue

            # Perform takeoff: update state variables
            planes -= 1
            takeoffs += 1
            total_takeoffs += 1

            # Signal if 5 takeoffs have occurred
            if takeoffs == 5:
                # Send SIGUSR1 to radio program
                radio_pid = shm_view[1]
                if radio_pid:
                    os.kill(radio_pid, signal.SIGUSR1)
                takeoffs = 0

        # Simulate takeoff time (1 second sleep)
        time.sleep(1)

        # Release the locked runway
        if runway == 1:
            runway1_lock.release()
        else:
            runway2_lock.release()

    # Send SIGTERM to radio program after completing max takeoffs
    with state_lock:
        if total_takeoffs >= TOTAL_TAKEOFFS:
            radio_pid = shm_view[1]
            if radio_pid:
                os.kill(radio_pid, signal.SIGTERM)


def main():
    global radio_proc, shm_fd, shm_mem

    try:
        # 1. Create shared memory segment
        create_shared_memory()

        # 2. Launch radio program - use path relative to script location
        script_dir = os.path.dirname(os.path.abspath(__file__))
        radio_path = os.path.join(script_dir, "radio")
        if not os.path.exists(radio_path):
            # Try current directory as fallback
            radio_path = "./radio"
        radio_proc = subprocess.Popen(
            [radio_path, SHM_NAME.decode()],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )

        # 3. Save radio PID in shared memory (second position)
        shm_view[1] = radio_proc.pid
        # Give radio process a moment to start
        time.sleep(0.1)

        # 4. Configure SIGUSR2 signal handler
        signal.signal(signal.SIGUSR2, sigusr2_handler)

        # 5. Launch 5 controller threads
        threads = [threading.Thread(target=takeoff_worker) for _ in range(STRIPS)]
        for t in threads:
            t.start()

        # 6. Wait for ground_control to attach (PID in shm[2])
        # print("Waiting for ground_control...", file=open("logs/air_control_py.log", "a"))
        while shm_view[2] == 0:
            time.sleep(0.1)

        # 7. Wait for all threads to complete
        for t in threads:
            t.join()

        # print(":::: End of operations ::::")
        # print(f"Takeoffs: {total_takeoffs} Planes: {planes + total_takeoffs}")
        # print(f"air pid: {shm_view[0]}")
        # print(f"radio pid: {shm_view[1]}")
        # print(f"ground pid: {shm_view[2]}")
        # print(":::::::::::::::::::::::::::")

    finally:
        # 8. Cleanup: close shared memory, close fd, and unlink
        if shm_mem:
            try:
                shm_mem.close()
            except:
                pass
        if shm_fd is not None and shm_fd != -1:
            try:
                _libc.close(shm_fd)
            except:
                pass
        try:
            _libc.shm_unlink(SHM_NAME)
        except:
            pass


if __name__ == "__main__":
    main()