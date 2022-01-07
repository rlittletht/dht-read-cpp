TARGETDIR=bin

all: | $(TARGETDIR) $(TARGETDIR)/test_dht_read

$(TARGETDIR):
	echo Making target directory $@
	-mkdir -p $@

PIDIR = pi
PI2DIR = pi2

PIFILES = $(PIDIR)/pi_dht_read.c $(PIDIR)/bcm2708.c
PI2FILES = $(PI2DIR)/pi_2_dht_read.c $(PI2DIR)/pi_2_mmio.c

$(TARGETDIR)/test_dht_read: test_dht_read.c $(PIFILES) $(PI2FILES) realtime.c
	gcc -o $@ -W -Wall -lrt $^

clean:
	rm -f $(TARGETDIR)/test_dht_read

