BINARY			:= bl_noah
KERNEL			:= /lib/modules/$(shell uname -r)/build
ARCH				:= x86
C_FLAGS			:= -Wall -O2
KMOD_DIR		:= $(shell pwd)
TARGET_PATH := /lib/modules/$(shell uname -r)/kernel/drivers/char
CONFIG_MODULE_SIG=n

OBJECTS := noah_driver.o

ccflags-y += $(C_FLAGS)

obj-m += $(BINARY).o

$(BINARY)-y := $(OBJECTS)

$(BINARY).ko:
	make -C $(KERNEL) M=$(KMOD_DIR) modules
	strip --strip-debug $(BINARY).ko

load:
	insmod ../build/$(BINARY).ko
	
unload:
	rmmod $(BINARY)
	
install:
	cp ../build/$(BINARY).ko $(TARGET_PATH)
	depmod -a

uninstall:
	rm $(TARGET_PATH)/$(BINARY).ko
	depmod -a

clean:
	make -C $(KERNEL) M=$(KMOD_DIR) clean
	