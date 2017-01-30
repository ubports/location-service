# CLI

locationd offers a command-line interface for controlling
and monitoring the service. The following commands are available:

 - `list`: Lists all provider implementations known to the service.
 - `monitor`: Connects to a locationd instance, monitoring its activity.
 - `provider`: Executes a known provider implementation in an out-of-process sandbox.
 - `run`: Executes the service.
 - `status`: Queries the status of a service instance.
 - `test`: Executes runtime tests against known provider implementations.

For all of the commands, an exit status of 0 indicates success. An exit status of 1 indicates
an error. Normal output goes to stdout, while all errors/warnings are output to stderr.

## Snap-Specific Command Names
If you are using the cli from a snap (`snap install locationd --channel edge`), the commands will 
be wrapped up for you in a convenient way, following the pattern locationd.$COMMAND. With that, if 
you want to check on the status of the service, simply run:
~~~bash
$ locationd.status
~~~

## Testing Scenarios
For testing purposes, it is often handy to inspect position/velocity/heading estimates on the command line.
The `monitor` command helps here. It connects to the service, starts the positioning engine and outputs 
position estimates to stdout until it receives a SIGTERM.

