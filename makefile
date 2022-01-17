
############### Directory definitions

TARGETDIR=bin
OBJDIR=obj

PIDIR = pi
PI2DIR = pi2
SRCDIR = .
THREADDIR = thread
OBJDIRS=$(OBJDIR)/.dir $(OBJDIR)/$(PIDIR)/.dir $(OBJDIR)/$(PIDIR)/.dir $(OBJDIR)/$(THREADDIR)/.dir

############### Compile flags
CC=g++
CXX=g++
CPPFLAGS=-W -Wall -std=c++17

############### Target files
PIFILES = $(OBJDIR)/pi/pi_dht_read.o $(OBJDIR)/pi/bcm2708.o
PI2FILES = $(OBJDIR)/pi_2_dht_read.o $(OBJDIR)/pi_2_mmio.o $(OBJDIR)/dht.o $(OBJDIR)/mmio.o
COREFILES = $(OBJDIR)/realtime.o
THREADFILES = $(OBJDIR)/PiThread.o $(OBJDIR)/timer.o

OBJFILES = $(OBJDIR)/test_dht_read.o $(PIFILES) $(COREFILES) $(PI2FILES) $(THREADFILES)

VPATH=$(SRCDIR):$(PIDIR):$(PI2DIR):$(OBJDIR):$(THREADDIR)

############### Inference Rules
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

############### Target rules

.PHONY: clean
.PHONY: all

all: | $(TARGETDIR) $(OBJDIRS) $(TARGETDIR)/test_dht_read

clean:
	rm -rf $(TARGETDIR)/*
	rm -rf $(OBJDIR)/*

$(OBJDIR):
	echo Making object directory $@
	-mkdir -p $@

%.dir:
	-mkdir -p $@

$(TARGETDIR):
	echo Making target directory $@
	-mkdir -p $@

$(TARGETDIR)/test_dht_read: $(OBJFILES)
	$(CC) -o $@ -W -Wall -lrt -lstdc++ $^

#note the doubling of the $ in the sed expression -- they are expanded twice. once by make and once by sh
.PHONY: compdb
compdb: clean $(OBJDIRS) $(OBJFILES:.o=.o.json)
	sed -e '1s/^/[\n/' -e '$$s/,$$/\n]/' $(OBJDIR)/compdb.int.json > compile_commands.json


