TARGETDIR=bin
OBJDIR=obj
CC=gcc
CXX=gcc
CPPFLAGS=-W -Wall

PIDIR = pi
PI2DIR = pi2
SRCDIR = .

PIFILES = $(OBJDIR)/pi_dht_read.o $(OBJDIR)/bcm2708.o
PI2FILES = $(OBJDIR)/pi_2_dht_read.c $(OBJDIR)/pi_2_mmio.c
COREFILES = $(OBJDIR)/realtime.o

VPATH=$(SRCDIR):$(PIDIR):$(PI2DIR):$(OBJDIR)

%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(OBJDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(OBJDIR)/%.o.json: %.c
	clang++ -MJ $@ -Wall -std=c++11 -o $(@:.o.json=.o) -c $<
	cat $@ >> $(OBJDIR)/compdb.int.json

.PHONY: clean
.PHONY: all

all: | $(TARGETDIR) $(OBJDIR) $(TARGETDIR)/test_dht_read

clean:
	rm -f $(TARGETDIR)/*
	rm -f $(OBJDIR)/*


OBJFILES = $(OBJDIR)/test_dht_read.o $(PIFILES) $(COREFILES) # $(PI2FILES)

$(OBJDIR):
	echo Making object directory $@
	-mkdir -p $@

$(TARGETDIR):
	echo Making target directory $@
	-mkdir -p $@

$(TARGETDIR)/test_dht_read: $(OBJFILES)
	gcc -o $@ -W -Wall -lrt $^

#note the doubling of the $ in the sed expression -- they are expanded twice. once by make and once by sh
.PHONY: compdb
compdb: clean $(OBJFILES:.o=.o.json)
	sed -e '1s/^/[\n/' -e '$$s/,$$/\n]/' $(OBJDIR)/compdb.int.json > $(TARGETDIR)/compile_commands.json


