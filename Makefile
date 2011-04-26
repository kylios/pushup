SRCFOLDER= src/
SOURCES= $(SRCFOLDER)lib/mongoose.o 

MAINSRC= $(SOURCES) $(SRCFOLDER)main.o

CFLAGS= -Iinclude -Wall -std=c99
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
