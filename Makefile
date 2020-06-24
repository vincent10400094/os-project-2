all:
	make -C /ksocket
	insmod ksocket.ko

	make -C /master_device
	insmod master_device.ko

	make -C /slave_device
	insmod slave_device.ko 

	make -C user_program/
	cp user_program/master master
	cp user_program/slave slave

clean:
	make clean -C user_program/
	rm slave
	rm master

	make clean -C /ksocket
	rmmod ksocket.ko

	make clean -C /master_device
	rmmod master_device.ko

	make clean -C /slave_device
	rmmod slave_device.ko