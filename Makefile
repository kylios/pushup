SRCFOLDER= src
SOURCES= $(SRCFOLDER)/lib/mongoose.o \
		 $(SRCFOLDER)/functions.o \
		 $(SRCFOLDER)/lib/list.o \
		 $(SRCFOLDER)/lib/hash.o \
		 $(SRCFOLDER)/protocol.o \
		 $(SRCFOLDER)/debug.o \
		 $(SRCFOLDER)/event.o \
		 $(SRCFOLDER)/user.o \
		 $(SRCFOLDER)/session.o \
		 $(SRCFOLDER)/event_queue.o

MAINSRC= $(SOURCES) $(SRCFOLDER)/main.o
TESTSRC= $(SOURCES) $(SRCFOLDER)/test.o

TESTFLAGS= -g -save-temps -D WITH_ASSERTS
CFLAGS= -Iinclude -Wall -std=c99 $(TESTFLAGS)
LDFLAGS= -pthread -ldl

EXE= pushup

#.SUFFIXES:
#.SUFFIXES: .o .c

all: $(MAINSRC)
	gcc $(LDFLAGS) -o $(EXE) $(MAINSRC)

test: $(TESTSRC)
	gcc $(LDFLAGS) -o $(EXE) $(TESTSRC)

src/main.o: $(SRCFOLDER)/main.c
	gcc $(CFLAGS) -c -o $(SRCFOLDER)/main.o $(SRCFOLDER)/main.c

src/test.o: $(SRCFOLDER)/test.c
	gcc $(CFLAGS) $(TESTFLAGS) -c -o $(SRCFOLDER)/test.o $(SRCFOLDER)/test.c

clean:
	rm $(EXE)
	rm $(SRCFOLDER)/*.o
	rm $(SRCFOLDER)/lib/*.o
	rm *.i *.s *.o

#.c.o:
#	$(CC) $(CFLAGS) $(TESTFLAGS) -c $<
