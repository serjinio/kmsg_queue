# Sample Linux kernel module

Implements a message queue via `/proc` fielsystem file object.
Supports text messages writing to and reading from the file object.
Messages are kept in a linked list structure which mimics FIFO queue.
On reading the first message which entered the queue will appear first on output.


## Build 

To build go to `src` folder and run `make`. 
Also can use `build.sh` warpper script which makes the same actions 
and additionally copies kernel module object to `bin` folder.

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


