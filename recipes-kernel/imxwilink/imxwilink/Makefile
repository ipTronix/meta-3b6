obj-m += imxwilink.o
# imxwilinkdrv-objs := imxwilink.o
obj-m += wilink_bt.o
# wilink_btdrv-objs := wilink_bt.o


SRC := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules_install

clean:
	rm -f *.o *~ core .depend .*.cmd *.ko *.mod.c
	rm -f Module.markers Module.symvers modules.order
	rm -rf .tmp_versions Modules.symvers
