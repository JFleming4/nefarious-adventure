# Assignment 1


## CHANGELOG

### v0.1.0

+ server.c
+ client.c
+ message.h

Messages contain a packet with a PID and a Text field.

The server reads messages with type 1 and puts a modified message back with type equal to the PID of the client that will read it back from the queue.
