obj-m := aufs.o
aufs-objs := super.o inode.o

CFLAGS_super.o := -DDEBUG
CFLAGS_inode.o := -DDEBUG

all:
	    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
