# make lib: create library
# make clean: remove .o .a files


#CC=gcc -D_POSIX_C_SOURCE=200112L -std=c99 -pedantic -fstrict-aliasing \
#         -Wall -Wno-long-long -Werror -D_NO_POSIX_LIBS

CC=gcc -D_POSIX_C_SOURCE=200112L -std=c99 -pedantic -fstrict-aliasing \
         -Wall -Wno-long-long -Werror


AR=ar

INCLUDE=./include/*.h
INCDIR= ./include
LIBDIR= ./lib
SRCDIR= ./src
BUILDDIR= ./build

all:			bdio.o md5.o $(LIBDIR)
			$(AR) -r $(BUILDDIR)/libbdio.a $(BUILDDIR)/bdio.o
			ranlib $(BUILDDIR)/libbdio.a; \
			$(AR) -r  $(BUILDDIR)/libmd5.a $(BUILDDIR)/md5.o
			ranlib $(BUILDDIR)/libmd5.a; \
			cp $(BUILDDIR)/libbdio.a $(BUILDDIR)/libmd5.a $(LIBDIR)/

bdio.o:			$(SRCDIR)/bdio.c $(INCLUDE) $(BUILDDIR)
			$(CC) -c $(SRCDIR)/bdio.c -o $(BUILDDIR)/bdio.o -I$(INCDIR)

md5.o:                  $(SRCDIR)/md5.c $(INCLUDE) $(BUILDDIR)
			$(CC) -c $(SRCDIR)/md5.c -o $(BUILDDIR)/md5.o -I$(INCDIR)

$(LIBDIR):		
			mkdir -p $(LIBDIR)

$(BUILDDIR):		
			mkdir -p $(BUILDDIR)

clean:		
			rm -f $(BUILDDIR)/*.o $(BUILDDIR)/libbdio.a $(BUILDDIR)/libmd5.a
