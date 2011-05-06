SRCFOLDER= src/
SOURCES= $(SRCFOLDER)lib/mongoose.o \
		 $(SRCFOLDER)lib/list.o \
		 $(SRCFOLDER)lib/hash.o \
		 $(SRCFOLDER)protocol.o \
		 $(SRCFOLDER)debug.o \
		 $(SRCFOLDER)event.o \
		 $(SRCFOLDER)user.o \
		 $(SRCFOLDER)session.o \
		 $(SRCFOLDER)event_queue.o

MAINSRC= $(SOURCES) $(SRCFOLDER)main.o

CFLAGS= -Iinclude -Wall -std=c99 -g #-save-temps
LDFLAGS= -pthread -ldl

EXE= pushup

all: $(MAINSRC)
	gcc $(LDFLAGS) -o $(EXE) $(MAINSRC)

src/main.o: $(SRCFOLDER)main.c
	gcc $(CFLAGS) -c -o $(SRCFOLDER)main.o $(SRCFOLDER)main.c

clean:
	rm $(EXE)
	rm $(SRCFOLDER)/*.o
	rm $(SRCFOLDER)/lib/*.o
