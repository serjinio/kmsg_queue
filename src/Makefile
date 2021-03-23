obj-m += queue.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -pthread -o test_driver test_driver.c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
