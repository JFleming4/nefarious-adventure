# Assignment 1

@author Derek Stride 100955939
@author Justin Fleming 100934170

## Execution

You can run a sample simulation with the following commands.

```bash
make
./main
```

The Makefile creates 3 executables, device, controller, and cloud.

To run the program you must start cloud, then controller, followed by creating some devices all in separate terminals. Device takes 3 arguments name, type and threshold. The type can be 0 for sensor or 1 for actuator.

The sensors generate a random number from 0 - 49 so make sure your threshold is within that range.

```bash
./cloud
./controller
./device "Kitchen Temp" 0 20
./device "Kitchen Heater" 1 x
./device "Stove Burner" 0 35
./device "Smoke Alarm" 1 x
```

## CHANGELOG

### v1.0.0

Added Main Function to run a simulation.

### v0.3.1

Signals propagate due to sweet sciency magic.

### v0.3.0

+ cloud.c
+ server.c -> controller.c
+ client.c -> device.c
+ message.c

The Makefile creates 3 executables, device, controller, and cloud.

To run the program you must start cloud, then controller, followed by creating some devices all in separate terminals. Device takes 3 arguments name, type and threshold. The type can be 0 for sensor or 1 for actuator.

The sensors generate a random number from 0 - 49 so make sure your threshold is within that range.

```bash
./cloud
./controller
./device "Kitchen Temp" 0 20
./device "Kitchen Heater" 1 x
./device "Stove Burner" 0 35
./device "Smoke Alarm" 1 x
```

### v0.1.0

+ server.c
+ client.c
+ message.h

Messages contain a packet with a PID and a Text field.

The server reads messages with type 1 and puts a modified message back with type equal to the PID of the client that will read it back from the queue.
