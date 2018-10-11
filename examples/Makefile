# make examples: build examples
# make clean: remove binary files

CC=gcc -g -std=c89 -pedantic -fstrict-aliasing \
         -Wall -Wno-long-long



INCDIR= ../include
LIBDIR= ../lib

examples:		lsbdio ./ex0/ex0 ./ex1/ex1 ./ex3/ex3_1 ./ex3/ex3_2 ./ex2/ex2
			
./ex0/ex0:		./ex0/ex0.c $(INCDIR)/*.h $(LIBDIR)/libbdio.a
			$(CC)  ./ex0/ex0.c -o ./ex0/ex0 -I$(INCDIR) -L$(LIBDIR) -lbdio

./ex1/ex1:		./ex1/ex1.c $(INCDIR)/*.h $(LIBDIR)/libbdio.a
			$(CC)  ./ex1/ex1.c -o ./ex1/ex1 -I$(INCDIR) -L$(LIBDIR) -lbdio -lm

./ex2/ex2:		./ex2/ex2.c $(INCDIR)/*.h $(LIBDIR)/libbdio.a
			$(CC)  ./ex2/ex2.c -o ./ex2/ex2 -I$(INCDIR) -L$(LIBDIR) -lbdio -lssl
./ex3/ex3_1:		./ex3/ex3_1.c $(INCDIR)/*.h $(LIBDIR)/libbdio.a
			$(CC) ./ex3/ex3_1.c -o ./ex3/ex3_1 -I$(INCDIR) -L$(LIBDIR) -lbdio
./ex3/ex3_2:		./ex3/ex3_2.c $(INCDIR)/*.h $(LIBDIR)/libbdio.a
			$(CC) ./ex3/ex3_2.c -o ./ex3/ex3_2 -I$(INCDIR) -L$(LIBDIR) -lbdio

clean:		
			rm -f ./ex0/ex0; \
                        rm -f ./ex1/ex1; \
                        rm -f ./ex2/ex2; \
                        rm -f ./ex3/ex3_1; \
                        rm -f ./ex3_ex3_2
lsbdio:			../tools/lsbdio
			cp ../tools/lsbdio ./ex0/; \
                        cp ../tools/lsbdio ./ex1/; \
                        cp ../tools/lsbdio ./ex2/; \
                        cp ../tools/lsbdio ./ex3/

