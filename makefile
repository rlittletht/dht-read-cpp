TARGETDIR=bin

all: | $(TARGETDIR) $(TARGETDIR)/test_dht_read

$(TARGETDIR):
	echo Making target directory $@
	-mkdir -p $@

PIDIR = pi
PIFILES = $(PIDIR)/pi_dht_read.c $(PIDIR)/bcm2708.c

$(TARGETDIR)/test_dht_read: test_dht_read.c $(PIFILES) realtime.c
	gcc -o $@ -W -Wall -lrt $^

clean:
	rm -f $(TARGETDIR)/test_dht_read

