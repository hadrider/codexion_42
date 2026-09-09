*This project has been created as part of the 42 curriculum by hadrider*

# Description

`codexion` is a concurrent simulation of coders competing for shared USB dongles.
Each coder is a POSIX thread. To compile, a coder must hold the two dongles next
to them, then they debug and refactor before trying to compile again.

The program supports two dongle arbitration policies:

- `fifo`: first request wins.
- `edf`: earliest burnout deadline wins. Equal deadlines prefer the higher coder id.

The implementation uses a small custom binary heap for each dongle, one mutex and
condition variable per dongle, a monitor thread for burnout detection, and mutexes
for shared state, logging, and request ordering.

# Instructions

Build:

```sh
make
```

Run:

```sh
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

Example:

```sh
./codexion 5 1500 200 100 100 5 50 fifo
```

The seven numeric arguments must be positive decimal integers. The scheduler must
be exactly `fifo` or `edf`.

# Blocking cases handled

## Deadlock

Coders take their first dongle in opposite directions depending on coder parity.
If the second dongle is unavailable, the first one is released. This avoids the
classic circular wait while allowing several non-adjacent coders to compile at
the same time. For five coders, the ring can therefore use two compiler pairs at
once instead of forcing a single pair through a random-looking order.

## Starvation

Every dongle has its own request heap. FIFO compares request arrival order. EDF
compares burnout deadlines and, when deadlines are equal, selects the higher
coder id as required by the evaluation recode.

## Cooldown

After a dongle is released, `available_at` is set to the current time plus the
configured cooldown. A request cannot acquire the dongle before that time.

## Burnout

A dedicated monitor checks every coder every millisecond. A coder burns out when
it has not started a new compile within `time_to_burnout` milliseconds from the
start of the simulation or its previous compile. The monitor prints the burnout
message and wakes all waiting threads.

## Successful termination

The monitor stops the simulation as soon as every coder has reached
`number_of_compiles_required`.

## Log serialization

All complete log lines are printed while holding `log_mutex`, so two threads
cannot mix their output on the same line.

# Thread synchronization mechanisms

- `pthread_mutex_t state_mutex` protects the stop flag, compile counters, and
  last compile timestamps.
- `pthread_mutex_t log_mutex` serializes output.
- `pthread_mutex_t order_mutex` assigns a unique request order.
- Each dongle owns a mutex protecting its owner, cooldown timestamp, and heap.
- Each dongle also owns a condition variable so waiting coders can sleep instead
  of continuously spinning.
- The monitor broadcasts all dongle conditions when the simulation ends, so no
  waiting coder remains blocked during shutdown.

# Resources

- POSIX threads and mutex/condition-variable documentation.
- Dining Philosophers synchronization and deadlock concepts.
- The Codexion subject and its peer-evaluation requirements.

AI was used as a review and debugging assistant to simplify the synchronization
logic, check edge cases, improve deterministic scheduling, and review the source
against the subject. The final code should be understood and defended by the
student during evaluation.
