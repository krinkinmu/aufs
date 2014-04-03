obj-m := aufs.o
aufs-objs := super.o

CFLAGS_super.o := -DDEBUG

all:
	    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
