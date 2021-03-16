# Sample Linux kernel module

Implements a message queue via `/proc` fielsystem file object.
Supports text messages writing to and reading from the file object.
Messages are kept in a linked list structure which mimics FIFO queue.
On reading the first message which entered the queue will appear first on output.


## Build 

To build go to `src` folder and run `make`

## Run 

To install module run: `sudo insmod queue.ko` from `src` folder.

Once module use is finished run: `sudo rmmod queue`.

