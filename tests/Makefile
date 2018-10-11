# make all: build test 
# make clean: remove binary files

CC=gcc -pedantic -fstrict-aliasing \
         -Wall -Wno-long-long -Werror 



INCDIR= ../include
LIBDIR= ../lib

all:			test testopen testread testappend testlongrec testhash

test:			testbdio.c $(INCDIR)/*.h $(LIBDIR)/libbdio.a
			$(CC)  testbdio.c -o testbdio -I$(INCDIR) -L$(LIBDIR) -lbdio -lmd5

testopen:		testopen.c $(INCDIR)/*.h $(LIBDIR)/libbdio.a
			$(CC) testopen.c -o testopen -I$(INCDIR) -L$(LIBDIR) -lbdio -lmd5

testread:               testread.c $(INCDIR)/*.h $(LIBDIR)/libbdio.a
			$(CC) testread.c -o testread -I$(INCDIR) -L$(LIBDIR) -lbdio -lmd5

testappend:             testappend.c $(INCDIR)/*.h $(LIBDIR)/libbdio.a
			$(CC) testappend.c -o testappend -I$(INCDIR) -L$(LIBDIR) -lbdio -lmd5
testlongrec:		testlongrec.c $(INCDIR)/*.h $(LIBDIR)/libbdio.a
			$(CC) testlongrec.c -o testlongrec -I$(INCDIR) -L$(LIBDIR) -lbdio -lmd5
testhash:		testhash.c $(INCDIR)/*.h $(LIBDIR)/libbdio.a $(LIBDIR)/libmd5.a
			$(CC) testhash.c -o testhash -I$(INCDIR) -L$(LIBDIR) -lbdio -lmd5



clean:		
			rm -f testbdio \
                        rm -f testread \
                        rm -f testopen \
                        rm -f testappend\
                        rm -f testlongrec\
                        rm -f testhash

