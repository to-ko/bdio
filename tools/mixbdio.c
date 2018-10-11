/* mixbdio.c
 *
 * mixbdio file1 n1 n2 file2 n3 n4 file3
 *
 * write a bdio file3 that has the header of file1 and contains
 * records n1-n2 of file1 followed by records n3-n4 of file2
 * 
 * Tomasz Korzec 2016
 ******************************************************************************/


#include <bdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <inttypes.h>

int main(int argc, char **argv)
{
   BDIO *f1,*f2,*f3;
   int i,n1,n2,n3,n4;
   size_t len,rd;
   char *buf;

   if (argc!=8)
   {
      fprintf(stderr,"usage: %s <in-file1> n1 n2 <in-file2> n3 n4 <out-file>\n",argv[0]);
      fprintf(stderr,"   writes a bdio out-file that has the header of in-file1\n");
      fprintf(stderr,"   and contains records n1-n2 of in-file1 followed by\n");
      fprintf(stderr,"   records n3-n4 of in-file2.\n\n");
      exit(EXIT_FAILURE);
   }

   if((f1 = bdio_open( argv[1], "r", NULL ))==NULL)
   {
      fprintf(stderr,"Could not open %s for reading\n",argv[1]);
      exit(EXIT_FAILURE);
   }

   if((f2 = bdio_open( argv[4], "r", NULL ))==NULL)
   {
      fprintf(stderr,"Could not open %s for reading\n",argv[4]);
      exit(EXIT_FAILURE);
   }

   if((f3 = bdio_open( argv[7], "w", bdio_get_hpinfo(f1) ))==NULL)
   {
      fprintf(stderr,"Could not open %s for writing\n",argv[7]);
      exit(EXIT_FAILURE);
   }

   /*TODO: conversion checks*/
   n1 = atoi(argv[2]);
   n2 = atoi(argv[3]);
   n3 = atoi(argv[5]);
   n4 = atoi(argv[6]);

   printf("Writing records %d-%d from %s and %d-%d from %s to %s\n",
      n1,n2,argv[1],n3,n4,argv[4],argv[7]);
   for (i=1;i<n1;i++)
   {
      if(bdio_seek_record(f1)==EOF)
      {
         fprintf(stderr,"Unexpected and of file");
         exit(EXIT_FAILURE);
      }
   }
   for (i=n1; i<=n2; i++)
   {
      if(bdio_seek_record(f1)==EOF)
      {
         fprintf(stderr,"Unexpected and of file");
         exit(EXIT_FAILURE);
      }
      bdio_start_record(bdio_get_rfmt(f1),bdio_get_ruinfo(f1),f3);
      len=bdio_get_rlen(f1);
      buf=malloc(len);
      rd=bdio_read(buf,len,f1);
      if(rd!=len)
      {
         fprintf(stderr,"Read error from infile1\n");
         exit(EXIT_FAILURE);
      }
      rd=bdio_write(buf,len,f3);
      if(rd!=len)
      {
         fprintf(stderr,"Write error to outfile\n");
         exit(EXIT_FAILURE);
      }
      free(buf);
   }
   for (i=1;i<n3;i++)
   {
      if(bdio_seek_record(f2)==EOF)
      {
         fprintf(stderr,"Unexpected and of file");
         exit(EXIT_FAILURE);
      }
   }
   for (i=n3; i<=n4; i++)
   {
      if(bdio_seek_record(f2)==EOF)
      {
         fprintf(stderr,"Unexpected and of file");
         exit(EXIT_FAILURE);
      }
      bdio_start_record(bdio_get_rfmt(f2),bdio_get_ruinfo(f2),f3);
      len=bdio_get_rlen(f2);
      buf=malloc(len);
      rd=bdio_read(buf,len,f2);
      if(rd!=len)
      {
         fprintf(stderr,"Read error from infile2\n");
         exit(EXIT_FAILURE);
      }
      rd=bdio_write(buf,len,f3);
      if(rd!=len)
      {
         fprintf(stderr,"Write error to outfile\n");
         exit(EXIT_FAILURE);
      }
      free(buf);
   }
   bdio_close(f1);
   bdio_close(f2);
   bdio_close(f3);
   return(EXIT_SUCCESS);
}