#
#	Makefile for cmmb, used PICL compiler.
#


#CC      = ./../picc
BIN     = D:/picc/9.80/bin
CC      = $(BIN)/picc

#CHIP    = --chip=16F690
#CHIP    = --chip=16F684
CHIP    = --chip=16F73


CFLAGS  = --asmlist --summary=psect,mem  -p --outdir=obj
SOURCE  =  ./aomcu_nofp.c ./aomcu_fd650_original.c ./aomcu_fd650_keyadc.c ./aomcu_ct1642_original.c ./aomcu_nofp_original.c ./aomcu_osc_original.c \
           ./aomcu_gpio_original.c ./pic16f7x.h ./aomcu_pt6393_original.c ./aomcu_original_str.c ./aomcu_aip1944_original.c

cmmb.rom: $(SOURCE)
	mkdir -p obj

	$(CC) $(CHIP) $(CFLAGS) "./aomcu_nofp.c" -G -M -O"nofp.hex"
	$(CC) $(CHIP) $(CFLAGS) "./aomcu_fd650_original.c" -G -M -O"fd650_original.hex"
	$(CC) $(CHIP) $(CFLAGS) "./aomcu_ct1642_original.c" -G -M -O"ct1642_original.hex"
	$(CC) $(CHIP) $(CFLAGS) "./aomcu_gpio_original.c" -G -M -O"gpio_original.hex"
	$(CC) $(CHIP) $(CFLAGS) "./aomcu_osc_original.c" -G -M -O"osc_original.hex"
	$(CC) $(CHIP) $(CFLAGS) "./aomcu_nofp_original.c" -G -M -O"nofp_original.hex"
	$(CC) $(CHIP) $(CFLAGS) "./aomcu_original_str.c" -G -M -O"aomcu_original_str.hex"
	$(CC) $(CHIP) $(CFLAGS) "./aomcu_tt1629.c" -G -M -O"aomcu_tt1629.hex"
	$(CC) $(CHIP) $(CFLAGS) "./aomcu_tt1629b.c" -G -M -O"aomcu_tt1629b.hex"
	$(CC) $(CHIP) $(CFLAGS) "./aomcu_pt6393_original.c" -G -M -O"aomcu_pt6393_original.hex"
	$(CC) $(CHIP) $(CFLAGS) "./aomcu_fd650_keyadc.c" -G -M -O"aomcu_fd650_keyadc.hex"
	$(CC) $(CHIP) $(CFLAGS) "./aomcu_aip1944_original.c" -G -M -O"aomcu_aip1944_original.hex"

	python hex2txt.py ./obj/nofp.hex ../aomcu_rom/aomcu_nofp.rom
	python hex2txt.py ./obj/fd650_original.hex ../aomcu_rom/aomcu_fd650_original.rom
	python hex2txt.py ./obj/ct1642_original.hex ../aomcu_rom/aomcu_ct1642_original.rom
	python hex2txt.py ./obj/gpio_original.hex ../aomcu_rom/aomcu_gpio_original.rom
	python hex2txt.py ./obj/osc_original.hex ../aomcu_rom/aomcu_osc_original.rom
	python hex2txt.py ./obj/nofp_original.hex ../aomcu_rom/aomcu_nofp_original.rom
	python hex2txt.py ./obj/aomcu_original_str.hex ../aomcu_rom/aomcu_original_str.rom
	python hex2txt.py ./obj/aomcu_tt1629.hex ../aomcu_rom/aomcu_tt1629.rom
	python hex2txt.py ./obj/aomcu_tt1629b.hex ../aomcu_rom/aomcu_tt1629b.rom
	python hex2txt.py ./obj/aomcu_pt6393_original.hex ../aomcu_rom/aomcu_pt6393_original.rom
	python hex2txt.py ./obj/aomcu_fd650_keyadc.hex ../aomcu_rom/aomcu_fd650_keyadc.rom
	python hex2txt.py ./obj/aomcu_aip1944_original.hex ../aomcu_rom/aomcu_aip1944_original.rom

	$(CC) $(CHIP) $(CFLAGS) --output=bin "./aomcu_nofp.c" -G -M -O"nofp.bin"
	$(CC) $(CHIP) $(CFLAGS) --output=bin "./aomcu_fd650_original.c" -G -M -O"fd650_original.bin"
	$(CC) $(CHIP) $(CFLAGS) --output=bin "./aomcu_ct1642_original.c" -G -M -O"ct1642_original.bin"
	$(CC) $(CHIP) $(CFLAGS) --output=bin "./aomcu_gpio_original.c" -G -M -O"gpio_original.bin"
	$(CC) $(CHIP) $(CFLAGS) --output=bin "./aomcu_osc_original.c" -G -M -O"osc_original.bin"
	$(CC) $(CHIP) $(CFLAGS) --output=bin "./aomcu_nofp_original.c" -G -M -O"nofp_original.bin"
	$(CC) $(CHIP) $(CFLAGS) --output=bin "./aomcu_original_str.c" -G -M -O"aomcu_original_str.bin"
	$(CC) $(CHIP) $(CFLAGS) --output=bin "./aomcu_tt1629.c" -G -M -O"aomcu_tt1629.bin"
	$(CC) $(CHIP) $(CFLAGS) --output=bin "./aomcu_tt1629b.c" -G -M -O"aomcu_tt1629b.bin"
	$(CC) $(CHIP) $(CFLAGS) --output=bin "./aomcu_pt6393_original.c" -G -M -O"aomcu_pt6393_original.bin"
	$(CC) $(CHIP) $(CFLAGS) --output=bin "./aomcu_fd650_keyadc.c" -G -M -O"aomcu_fd650_keyadc.bin"
	$(CC) $(CHIP) $(CFLAGS) --output=bin "./aomcu_aip1944_original.c" -G -M -O"aomcu_aip1944_original.bin"

	cp -af ./obj/*.bin ../aomcu_bin

clean:
	rm -rfv obj/*  test.txt *.rom *.bin
