# Sample Linux kernel module

Implements a message queue via `/proc` fielsystem file object.
Supports text messages writing to and reading from the file object.
Messages are kept in a linked list structure which mimics FIFO queue.
On reading the first message which entered the queue will appear first on output.


## Build 

To build go to `src` folder and run `make`

## Manual run 

To install module run: 

```
sudo insmod queue.ko
```

from `src` folder.

Once module use is finished run:

```
sudo rmmod queue
```

## Test

Run from project root:

```
test/test1.sh
```

Sample shell session output:

```
➜  kmsg_queue git:(kmsg_module_impl1) ✗ test/test1.sh 
Sending message: hello 1000000
Sending message: hello 2
Sending message: hello 3
Should obtain back three messages
hello 1000000
<MSG DELIM>
hello 2
<MSG DELIM>
hello 3
<MSG DELIM>
Now queue should be empty (no data after this message)
<MSG DELIM>

```


