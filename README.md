# Sample Linux kernel module

Implements a message queue via `/proc` fielsystem file object.
Supports text messages writing to and reading from the file object.
Messages are kept in a linked list structure which mimics FIFO queue.
On reading the first message which entered the queue will appear first on output.


## Build 

To build go to `src` folder and run `make`. 
Also can use `build.sh` warpper script which makes the same actions 
and additionally copies kernel module object to `bin` folder.

### Adjustable parameters

There are several `define ...` constants that could be changed in `queue.c`.

Change `MSG_MAX_SIZE` to set maximum allowed message size.
Change `PROCFS_NAME` to set the name of the file used in `/proc/` folder.
Change `PROCFS_FILE_MODE` to set the access mode of the procfs file.

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

Sample shell session that uses the module:

```
➜  kmsg_queue git:(kmsg_module_impl1) ✗ cd src
➜  src git:(kmsg_module_impl1) ✗ make
make -C /lib/modules/5.8.0-45-generic/build M=/home/serj/projects/lkmd/kmsg_queue/src modules
make[1]: Entering directory '/usr/src/linux-headers-5.8.0-45-generic'
make[1]: Leaving directory '/usr/src/linux-headers-5.8.0-45-generic'
➜  src git:(kmsg_module_impl1) ✗ sudo insmod queue.ko
➜  src git:(kmsg_module_impl1) ✗ echo "hello 1" > /proc/msg_queue 
➜  src git:(kmsg_module_impl1) ✗ echo "hello 2" > /proc/msg_queue
➜  src git:(kmsg_module_impl1) ✗ echo "hello 3" > /proc/msg_queue
➜  src git:(kmsg_module_impl1) ✗ cat /proc/msg_queue             
hello 1
➜  src git:(kmsg_module_impl1) ✗ cat /proc/msg_queue
hello 2
➜  src git:(kmsg_module_impl1) ✗ cat /proc/msg_queue
hello 3
➜  src git:(kmsg_module_impl1) ✗ cat /proc/msg_queue
➜  src git:(kmsg_module_impl1) ✗ 
```

## Tests

### Queue insert & pop

Run from project root:

```
test/test_insert_and_pop.sh
```

Sample shell session output:

```
➜  kmsg_queue git:(kmsg_module_impl1) ✗ test/test_insert_and_pop.sh 
*** Test for simple insert & pop ***
Sending message: hello 1
Sending message: hello 2
Sending message: hello 3
Should obtain back three messages
hello 1
<MSG DELIM>
hello 2
<MSG DELIM>
hello 3
<MSG DELIM>
Now queue should be empty (no data after this message)
<MSG DELIM>
*** END: Test for simple insert & pop ***
```

Run other tests to see their output.

### Queue cleanup

```
➜  kmsg_queue git:(kmsg_module_impl1) ✗ ./test/test_cleanup.sh 
*** Test for cleanup of left-over resources on rmmod call ***
Sending message: hello 1
Sending message: hello 2
Sending message: hello 3
Sending message: hello 1
Sending message: hello 2
Sending message: hello 3
Should obtain back three messages
hello 1
<MSG DELIM>
hello 2
<MSG DELIM>
hello 3
<MSG DELIM>
Now queue should still contain 3 messages - should be cleaned on rmmod
Last five lines from kern.log:
Mar 19 16:42:08 ubu kernel: [ 1901.771406] kmsg_queue: entering cleanup()
Mar 19 16:42:08 ubu kernel: [ 1901.771408] kmsg_queue: removed message of size 8
Mar 19 16:42:08 ubu kernel: [ 1901.771409] kmsg_queue: removed message of size 8
Mar 19 16:42:08 ubu kernel: [ 1901.771409] kmsg_queue: removed message of size 8
Mar 19 16:42:08 ubu kernel: [ 1901.771409] kmsg_queue: cleanup() finished
*** END: Test for cleanup of left-over resources on rmmod call ***
```

### Queue cleanup with large amount of messages


```
➜  kmsg_queue git:(kmsg_module_impl1) ✗ ./test/test_large_cleanup.sh 
*** Test for cleanup of left-over resources using many messages ***
Will put 1000 messages to the queue...
Will obtain back three messages
Welcome number 1
<MSG DELIM>
Welcome number 2
<MSG DELIM>
Welcome number 3
<MSG DELIM>
Now queue should still contain 997 messages - should be cleaned on rmmod
Last fifty lines from kern.log:
Mar 19 16:43:43 ubu kernel: [ 1996.946330] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946330] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946331] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946332] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946332] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946333] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946333] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946334] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946334] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946335] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946336] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946336] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946337] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946337] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946338] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946338] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946339] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946339] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946340] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946340] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946341] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946342] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946343] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946343] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946344] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946344] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946345] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946345] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946346] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946346] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946347] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946347] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946348] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946348] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946349] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946350] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946350] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946351] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946351] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946352] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946352] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946353] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946353] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946354] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946354] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946355] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946355] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946356] kmsg_queue: removed message of size 19
Mar 19 16:43:43 ubu kernel: [ 1996.946356] kmsg_queue: removed message of size 20
Mar 19 16:43:43 ubu kernel: [ 1996.946357] kmsg_queue: cleanup() finished
*** END: Test for cleanup of left-over resources on rmmod call ***
```

Number of message that are sent to the queue can be edited in test sources.

