# make tools: build tools
# make clean: remove binary files

CC=gcc -pedantic -fstrict-aliasing \
         -Wall -Wno-long-long



INCDIR= ../include
LIBDIR= ../lib

tools:			replacetag.c mixbdio.c lsbdio.c $(INCDIR)/*.h $(LIBDIR)/libbdio.a
			$(CC)  lsbdio.c -o lsbdio -I$(INCDIR) -L$(LIBDIR) -lbdio -lmd5
			$(CC)  mixbdio.c -o mixbdio -I$(INCDIR) -L$(LIBDIR) -lbdio -lmd5
			$(CC)  replacetag.c -o replacetag -I$(INCDIR) -L$(LIBDIR) -lbdio -lmd5
			$(CC)  cropbdio.c -o cropbdio -I$(INCDIR) -L$(LIBDIR) -lbdio -lmd5

clean:		
			rm -f lsbdio mixbdio replacetag
