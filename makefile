TARGETDIR=bin
OBJDIR=obj
CC=g++
CXX=g++
CPPFLAGS=-W -Wall -std=c++17

PIDIR = pi
PI2DIR = pi2
SRCDIR = .
THREADDIR = thread

PIFILES = $(OBJDIR)/pi_dht_read.o $(OBJDIR)/bcm2708.o
PI2FILES = $(OBJDIR)/pi_2_dht_read.o $(OBJDIR)/pi_2_mmio.o $(OBJDIR)/Pi2Dht.o

COREFILES = $(OBJDIR)/realtime.o
THREADFILES = $(OBJDIR)/PiClock.o $(OBJDIR)/PiTimer.o

VPATH=$(SRCDIR):$(PIDIR):$(PI2DIR):$(OBJDIR):$(THREADDIR)

%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(OBJDIR)/%.o: %.cc
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(OBJDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(OBJDIR)/%.o.json: %.c
	clang++ -MJ $@ -Wall -std=c++17 -o $(@:.o.json=.o) -c $<
	cat $@ >> $(OBJDIR)/compdb.int.json

$(OBJDIR)/%.o.json: %.cc
	clang++ -MJ $@ -Wall -std=c++17 -o $(@:.o.json=.o) -c $<
	cat $@ >> $(OBJDIR)/compdb.int.json

.PHONY: clean
.PHONY: all

all: | $(TARGETDIR) $(OBJDIR) $(TARGETDIR)/test_dht_read

clean:
	rm -f $(TARGETDIR)/*
	rm -f $(OBJDIR)/*


OBJFILES = $(OBJDIR)/test_dht_read.o $(PIFILES) $(COREFILES) $(PI2FILES) $(THREADFILES)

$(OBJDIR):
	echo Making object directory $@
	-mkdir -p $@

$(TARGETDIR):
	echo Making target directory $@
	-mkdir -p $@

$(TARGETDIR)/test_dht_read: $(OBJFILES)
	$(CC) -o $@ -W -Wall -lrt -lstdc++ $^

#note the doubling of the $ in the sed expression -- they are expanded twice. once by make and once by sh
.PHONY: compdb
compdb: clean $(OBJFILES:.o=.o.json)
	sed -e '1s/^/[\n/' -e '$$s/,$$/\n]/' $(OBJDIR)/compdb.int.json > compile_commands.json


