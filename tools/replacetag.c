/* replacetag.c
 *
 * replacetag file1 t1 t2 file2
 *
 * look for appearances of t1 in ascii records of bdio file1
 * and replace them with t2. Write the result to file2
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

void strrep(char *buf, char *tag1, char *tag2, size_t *len)
{
   int i,j;
   int tlen1,tlen2;
   int found=0;
   char *buf2;

   buf2=malloc(*len);
   memcpy(buf2,buf,*len);
   tlen1=strlen(tag1);
   tlen2=strlen(tag2);

   for (i=0; i<*len-tlen1; i++)
   {
      found=1;
      for (j=0; j<tlen1; j++)
      {
         if(buf[i+j]!=tag1[j])
         {
            found=0;
            break;
         }
      }
      if (found)
      {
         memcpy(buf+i,tag2,tlen2);
         memcpy(buf+i+tlen2,buf2+i+tlen1,*len-i-tlen1);
         *len = *len+tlen2-tlen1;
         free(buf2);
         return;
      }
   }
   return;
}


int main(int argc, char **argv)
{
   BDIO *f1,*f2;
   int rd;
   size_t len;
   char *buf;

   if (argc!=5)
   {
      fprintf(stderr,"usage: %s <in-file1> t1 t2 <out-file>\n",argv[0]);
      fprintf(stderr,"   writes a bdio out-file which is a copy of in-file1\n");
      fprintf(stderr,"   but occurances of t1 in ascii records are \n");
      fprintf(stderr,"   replaced by t2 and all but the first headers are stripped\n\n");
      exit(EXIT_FAILURE);
   }

   if((f1 = bdio_open( argv[1], "r", NULL ))==NULL)
   {
      fprintf(stderr,"Could not open %s for reading\n",argv[1]);
      exit(EXIT_FAILURE);
   }

   if((f2 = bdio_open( argv[4], "w", bdio_get_hpinfo(f1) ))==NULL)
   {
      fprintf(stderr,"Could not open %s for writing\n",argv[4]);
      exit(EXIT_FAILURE);
   }

   while(bdio_seek_record(f1)!=EOF)
   {
      if(bdio_is_in_record(f1))
      {
         bdio_start_record(bdio_get_rfmt(f1),bdio_get_ruinfo(f1),f2);
         len=bdio_get_rlen(f1);
         if(bdio_get_rfmt(f1)==BDIO_ASC_GENERIC)
         {
            buf=malloc(len+strlen(argv[3]));
            rd=bdio_read(buf,len,f1);
            if(rd!=len)
            {
               fprintf(stderr,"Read error from infile1\n");
               exit(EXIT_FAILURE);
            }
            strrep(buf,argv[2],argv[3],&len);
            rd=bdio_write(buf,len,f2);
            if(rd!=len)
            {
               fprintf(stderr,"Write error to outfile\n");
               exit(EXIT_FAILURE);
            }
            free(buf);
         }else
         {
            buf=malloc(len);
            rd=bdio_read(buf,len,f1);
            if(rd!=len)
            {
               fprintf(stderr,"Read error from infile1\n");
               exit(EXIT_FAILURE);
            }
            rd=bdio_write(buf,len,f2);
            if(rd!=len)
            {
               fprintf(stderr,"Write error to outfile\n");
               exit(EXIT_FAILURE);
            }
            free(buf);
         }
      }
   }
   bdio_close(f1);
   bdio_close(f2);
   return(EXIT_SUCCESS);
}