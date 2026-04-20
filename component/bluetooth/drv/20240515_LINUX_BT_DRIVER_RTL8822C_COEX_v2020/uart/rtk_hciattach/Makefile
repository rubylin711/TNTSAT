include ${SDK_DIR}/build/script/base.mk
CFLAGS := -Wall -g 

all: rtk_hciattach
OBJS := hciattach.o hciattach_rtk.o hciattach_h4.o rtb_fwc.o

rtk_hciattach: $(OBJS)
	$(CC) -o rtk_hciattach $(OBJS)
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(OBJS)  rtk_hciattach
install:
	cp -af rtk_hciattach $(BIN_DIR)/rtk_hciattach
uninstall:
	rm -rf $(BIN_DIR)/rtk_hciattach


