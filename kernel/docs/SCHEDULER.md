# The Scheduler

The scheduler is the component of the ZEBRA Kernel responsible for deciding which processes run and when.

Scheduling is done in a round-robin fashion, with each process being given a time-slice of one scheduler tick.

Each process can be in one of several states:

* `RUNNING` - Process is currently executing on the CPU
* `READY` - Process is ready to be executed when a time slice is available
* `BLOCKED` - Process is waiting on an event to occur
* `FINISHED` - Process has exited and is waiting to be cleaned up by the scheduler

The transitions between events is described below:

```
RUNNING -> scheduler tick -> READY
RUNNING -> wait syscall -> BLOCKED
RUNNING -> exit syscall -> FINISHED

READY -> scheduler tick -> RUNNING

BLOCKED -> waited event occurs -> READY
```

## Events

Events are stored as a single type byte, followed by a data field. For some types the data field is of a known
fixed type, whereas for others it is variable and can be preceeded by a length byte.
How the event field is interpreted is dependent on the event type.

The events on which a process can wait are described below:

* `PROCESS_FINISHED` - Another process has completed. The data field is two bytes indicating the PID of the completed process.
