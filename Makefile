SRCFOLDER= src/
SOURCES= $(SRCFOLDER)lib/mongoose.o \
		 $(SRCFOLDER)lib/list.o \
		 $(SRCFOLDER)protocol.o \
		 $(SRCFOLDER)debug.o \
		 $(SRCFOLDER)event.o \
		/usr/local/lib/libyajl_s.a
#		yajl/lloyd-yajl-a8f3663/build/yajl-2.0.0/bin/json_reformat \
#		yajl/lloyd-yajl-a8f3663/build/yajl-2.0.0/bin/json_verify

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
