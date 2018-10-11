/* ex2.c
 *
 * takes a file and creates a bdio file that contains
 * - the file's MD5 sum
 * - a compressed version of the file
 * - a script to extract the original file from the bdio file and compare the
 *   checksum
 * 
 * usage:
 *
 * ex2 <input-file> <output bdio-file>
 *
 * Tomasz Korzec 2014
 ******************************************************************************/


#include <bdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <unistd.h>

#define BUFSIZE 1000

unsigned char buf[BUFSIZE];

void md5_sum(char *fname, unsigned char * out)
{
   MD5_CTX c;
   ssize_t bytes;
   FILE *fp;

   if( (fp = fopen(fname,"r"))==NULL )
   {
      fprintf(stderr,"cannot open %s for reading.\n",fname);
      exit(EXIT_FAILURE);
   }

   MD5_Init(&c);
   bytes=fread(buf, 1, BUFSIZE, fp);
   while(bytes > 0)
   {
            MD5_Update(&c, buf, bytes);
            bytes=fread(buf, 1, BUFSIZE, fp);
   }

   MD5_Final(out, &c);
   fclose(fp);
}

int main(int argc, char *argv[])
{
   BDIO *fh;
   FILE *fp;
   
   int i, rd;

   char sh[] ="#!/bin/bash\n"
              "echo \"name of bdio file\"\n"
              "read fin \n"
              "echo \"name uncompressed file\"\n"
              "read fout \n"
              "lsbdio -d 3 $fin > ${fout}.gz\n"
              "gunzip ${fout}.gz\n"
              "echo \"uncompressed file has MD5 sum $(md5sum ${fout}) \"\n"
              "echo \"MD5 sum should be             $(lsbdio -d 2 ${fin})\"\n";
   unsigned char md5[MD5_DIGEST_LENGTH];
   char md5_str[MD5_DIGEST_LENGTH*2+1];
   char tmp[128];
   char zipname[128];

   if( argc != 3 )
   {
      printf("usage:\nex2 <input-file> <output-file>\n");
      exit(EXIT_FAILURE);
   }
   

   bdio_set_dflt_verbose(1);


   md5_sum(argv[1], md5);

   printf("%s has md5 sum: \n",argv[1]);
   for(i=0; i<MD5_DIGEST_LENGTH; i++)
      sprintf(&(md5_str[2*i]),"%02x", md5[i]);
   printf("%s\n",md5_str);

   printf("compressing input...\n");
   sprintf(zipname,"%s.gz",argv[1]);
   sprintf(tmp,"gzip %s -c > %s",argv[1],zipname);
   if( system(tmp) != 0 )
   {
      fprintf(stderr,"gzip failed\n");
      exit(EXIT_FAILURE);
   }

   /* open zip file for reading */
   if( (fp = fopen(zipname,"r"))==NULL )
   {
      fprintf(stderr,"cannot open %s for reading.\n",zipname);
      exit(EXIT_FAILURE);
   }
   
   /* open a bdio file for writing */
   if ((fh = bdio_open( argv[2], "w",
      "execute shell script in the first record to decompress"))==NULL)
      exit(EXIT_FAILURE);

   /* write record with shell script */
   if(bdio_start_record(BDIO_ASC_EXEC, 0, fh))
      exit(EXIT_FAILURE);

   if(bdio_write(sh, strlen(sh), fh)!=strlen(sh))
      exit(EXIT_FAILURE);

   /* write record with MD5 sum */
   if(bdio_start_record(BDIO_ASC_GENERIC, 0, fh))
      exit(EXIT_FAILURE);

   if(bdio_write(md5_str, strlen(md5_str), fh)!=strlen(md5_str))
      exit(EXIT_FAILURE);

   /* write record with zipped configuration */
   if(bdio_start_record(BDIO_BIN_GENERIC, 0, fh))
      exit(EXIT_FAILURE);

   rd=fread(buf, 1, BUFSIZE, fp);
   while(rd > 0)
   {
            if( bdio_write(buf, rd, fh) != rd )
               exit(EXIT_FAILURE);
            rd=fread(buf, 1, BUFSIZE, fp);
   }

   fclose(fp);
   if(bdio_close(fh))
      exit(EXIT_FAILURE);

   sprintf(tmp,"rm %s",zipname);
   system(tmp);
   exit(EXIT_SUCCESS);
}
