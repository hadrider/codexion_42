*This project has been created as part of the 42 curriculum by hadrider*

# Description

`codexion` is a Dining-Philosophers variant implemented with POSIX threads.
Each coder is a thread and each dongle is a shared resource. A coder repeatedly
compiles while holding both adjacent dongles, then debugs and refactors without
holding dongles.

The implementation uses per-dongle mutexes and condition variables, a hand-rolled
binary heap for arbitration, a dedicated monitor thread for burnout detection,
and a shared shutdown state.

# Instructions

Build:

```sh
make
```

Run:

```sh
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

`scheduler` must be `fifo` or `edf`. All numeric arguments must be positive
decimal integers.

Example:

```sh
./codexion 5 800 100 100 100 3 20 fifo
```

# Resources

- POSIX Threads: https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_create.html
- POSIX condition variables: https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_cond_wait.html
- POSIX mutexes: https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_lock.html
- Dining Philosophers background: https://en.wikipedia.org/wiki/Dining_philosophers_problem

AI was used as an implementation and review assistant for the project structure,
synchronization design, debugging, and documentation. The final source should
be understood and defended by the student rather than treated as unexplained
generated code.

# Blocking cases handled

## Deadlock

Coders acquire the two dongles in ascending global dongle ID order. Therefore
there cannot be a circular wait: every dependency points toward a higher
resource ID. This breaks the circular-wait Coffman condition structurally.

## Starvation

Requests are explicitly ordered in a per-dongle heap. FIFO uses arrival order.
EDF prioritizes the earliest compile deadline and uses arrival order and coder ID
as deterministic tie breakers.

## Cooldown

Every release sets `available_at` to the current time plus the configured
cooldown. A waiting coder remains blocked until both its request is the heap
winner and the cooldown timestamp has passed.

## Burnout

A dedicated monitor checks every coder's last compile start at a 1 ms polling
interval. Burnout is based on the deadline `last_compile_start + burnout`.
The monitor logs the event and initiates shutdown.

## Log serialization

Every complete log line is formatted before acquiring the logging mutex. The
single output operation is protected by that mutex, preventing interleaving.

# Thread synchronization mechanisms

## `state_mutex`

Protects the global `stop` flag and coder compile counters / monitor snapshots.
Without it, a coder could read stale shutdown state while another thread writes
it, or the monitor could race with compile-count updates.

## `log_mutex`

Serializes complete log writes. Without it, two threads could interleave their
output.

## `seq_mutex`

Protects the global request arrival sequence. Without it, simultaneous requests
could receive duplicate or inconsistent ordering numbers.

## Per-dongle `mutex`

Protects `owner`, `available_at`, and the pending request heap for that dongle.
Without it, two coders could both observe an available dongle and claim it.

## Per-dongle `cond`

Allows waiters to sleep until the dongle state changes or a cooldown deadline is
approached. The condition is always checked in a `while` loop after waking.

## Shutdown broadcasts

When shutdown starts, every dongle condition variable is broadcast so waiters
do not remain asleep waiting for a resource that will never become available.

# Validation

The intended validation commands include:

```sh
make
make re
valgrind --leak-check=full ./codexion 5 800 100 100 100 3 20 fifo
cc -Wall -Wextra -Werror -pthread -fsanitize=thread *.c -o codexion-tsan
```

# Notes

The source is intentionally split by responsibility: parsing, initialization,
heap arbitration, dongle state, coder behavior, monitoring, logging, and utility
functions. This makes the synchronization boundaries explicit for an oral
defense.
