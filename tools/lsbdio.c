/* lsbdio.c
 *
 * lists the content of a bdio file
 *
 * Tomasz Korzec 2013-2014
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

#define BDIO_BIN_GENERIC 0x00
#define BDIO_ASC_EXEC    0x01
#define BDIO_BIN_INT32BE 0x02
#define BDIO_BIN_INT32LE 0x03
#define BDIO_BIN_INT64BE 0x04
#define BDIO_BIN_INT64LE 0x05
#define BDIO_BIN_F32BE   0x06
#define BDIO_BIN_F32LE   0x07
#define BDIO_BIN_F64BE   0x08
#define BDIO_BIN_F64LE   0x09
#define BDIO_ASC_GENERIC 0x0A
#define BDIO_ASC_XML     0x0B

#define LSBDIO_VERSION "1.0"

char RSET[16], FREC[16], IREC[16], AREC[16], BREC[16], EREC[16];
char XREC[16], HDR[16], METC[16], CREC[16];

static char fmt[12][7] ={"bin   \0","exe   \0","i32 be\0","i32 le\0","i64 be\0",
                         "i64 le\0","f32 be\0","f32 le\0","f64 be\0","f64 le\0",
                         "ASCII \0","XML   \0"};
static char lrec[2][3] = {" -"," x"};

static char lenstr[8];

int data_flag=0, meta_flag=0, raw_flag=0;
int dindex=0;
int mindex=0;

void setcolor(char col)
{
   if( col )
   {
       strcpy(RSET,"\033[0m");
       strcpy(FREC,"\033[32;40m");
       strcpy(IREC,"\033[33;40m");
       strcpy(AREC,"\033[0;36;40m");
       strcpy(BREC,"\033[0;35;40m");
       strcpy(EREC,"\033[0;37;40m");
       strcpy(XREC,"\033[0;31;40m");
       strcpy(HDR, "\033[34;47m");
       strcpy(METC,"\033[1;34m");
       strcpy(CREC,"\033[1;31;40m");
   }else
   {
       strcpy(RSET,"");
       strcpy(FREC,"");
       strcpy(IREC,"");
       strcpy(AREC,"");
       strcpy(BREC,"");
       strcpy(EREC,"");
       strcpy(XREC,"");
       strcpy(HDR, "");
       strcpy(METC,"");
       strcpy(CREC,"");
   }
}

void strcleanup(unsigned char* s)
{
   while( *s!=0 )
   {
      if(*s=='\n')    s[0] = 182;
      else if(*s<32)  s[0] = 149;
      else if(*s>126) s[0] = 149;
      s++;
   }
}

void strcleanup2(char* s, int len)
{
   int i;
   for (i=0; i<len; i++)
   {
      if(*s=='\0')    s[0] = '\n';
      else if(*s<32)  s[0] = ' ';
      else if(*s>126) s[0] = ' ';
      s++;
   }
}

char *printlen(uint64_t len)
{
   if (len<10000000)
      sprintf(lenstr,"%7" PRIu64 ,len);
   else if (len<1000000000)
      sprintf(lenstr,"%6" PRIu64 "K",len/1000);
   else if (len<1000000000000)
      sprintf(lenstr,"%6" PRIu64 "M",len/1000000);
   else if (len<1000000000000000)
      sprintf(lenstr,"%6" PRIu64 "G",len/1000000000);
   else
      sprintf(lenstr,"%6" PRIu64 "T",len/1000000000000);

   return lenstr;
}

void print_record_meta(long id, BDIO *fh)
{
   struct tm * timeinfo;
   time_t t;
   char tstr[80];
   t = (time_t) bdio_get_hmdate(fh);
   timeinfo = localtime (&t);
   strftime (tstr,80,"%c",timeinfo);
   printf("%srecord no:%s  %i\n",METC,RSET,bdio_get_rcnt(fh));
   printf("%ssize:%s       %" PRIu64 " byte\n",METC,RSET,bdio_get_rlen(fh));
   /*printf(METC "rstart:     %li\n",bdio_get_rstart(fh));*/
   printf("%suser-info:%s  %i\n",METC,RSET,bdio_get_ruinfo(fh));
   printf("%swritten by:%s %s@%s\n",METC,RSET,bdio_get_hmuser(fh),bdio_get_hmhost(fh));
   printf("%swritten on:%s %s\n",METC,RSET,tstr);
   
   return;
}

void print_empty_header(long id)
{
   if( meta_flag && mindex==id)
   {
      printf("%s-------------------------------------------------------------------------------%s\n",METC,RSET);
      printf("%s%-6li irrelevant header ----------------------------------------------------- %s\n",HDR,id,RSET);
      printf("%s-------------------------------------------------------------------------------%s\n",METC,RSET);
   }
   if( (!meta_flag) && (!data_flag) )
   {
      printf("%s%-6li irrelevant header ----------------------------------------------------- %s\n",HDR,id,RSET);
   }
}

void print_header(long id, BDIO *fh)
{
   time_t tc, tm;
   tc = (time_t) bdio_get_hcdate(fh);
   tm = (time_t) bdio_get_hmdate(fh);
   if( meta_flag && mindex==id)
   {
      printf("%s-------------------------------------------------------------------------------%s\n",METC,RSET);
      printf("%s%-6li header v%2i %16.16s@%-16.16s %24.24s   %s\n"
      ,HDR,id,bdio_get_hversion(fh),bdio_get_hcuser(fh),bdio_get_hchost(fh),asctime(localtime(&tm)),RSET);
      printf("%sheader no:%s        %i\n",METC,RSET,bdio_get_hcnt(fh));
      printf("%sheader version:%s   %i\n",METC,RSET,bdio_get_hversion(fh));
      /*printf(METC "size:" RSET "             %li byte\n",bdio_get_rstart(fh)-bdio_get_hstart(fh));*/
      /*printf(METC "rstart:" RSET "     %li\n",bdio_get_rstart(fh));*/
      printf("%screated by:%s       %s@%s\n",METC,RSET,bdio_get_hcuser(fh),bdio_get_hchost(fh));
      printf("%slast modified by:%s %s@%s\n",METC,RSET,bdio_get_hmuser(fh),bdio_get_hmhost(fh));
      printf("%screated on:%s       %s",METC,RSET,asctime(localtime(&tc)));
      printf("%slast modified on:%s %s",METC,RSET,asctime(localtime(&tm)));
      printf("%sprotocol info:%s    %s\n",METC,RSET,bdio_get_hpinfo(fh));
      printf("%s-------------------------------------------------------------------------------%s\n",METC,RSET);
   }
   if( (!meta_flag) && (!data_flag) )
   {
      printf("%s%-6li header v%2i %16.16s@%-16.16s %24.24s   %s\n"
      ,HDR,id,bdio_get_hversion(fh),bdio_get_hcuser(fh),bdio_get_hchost(fh),asctime(localtime(&tm)),RSET);
   }
}


void print_record_int32(long id, BDIO *fh)
{
   int d[13];
   char str[13*12+1];
   char tmpstr[12+1];
   int i;
   size_t rd = 13*4, rd2;
   int32_t *dat;

   if( data_flag && dindex==id)
   {
      rd2 = bdio_get_rlen(fh);
      dat = (int32_t*) malloc(rd2);
      bdio_read_int32(dat,rd2,fh);
   }
   if( meta_flag && mindex==id)
   {
      printf("%s-------------------------------------------------------------------------------%s\n",METC,RSET);
      if( bdio_get_rlen(fh)<rd )
         rd=bdio_get_rlen(fh);
      if( data_flag && dindex==id )
         memcpy(d,dat,rd);
      else
         rd=bdio_read_int32(d,rd,fh);
      for (i=0; i<13*12+1; i++)
         str[i] = ' ';
      str[0]='\0';
      for( i=0; i<rd/4; i++)
      {
         sprintf(tmpstr,"%i ",d[i]);
         if( strlen(tmpstr)+strlen(str)<=40 )
            strcat(str,tmpstr);
      }
      printf("%s%-6li record %s %s byte %2i %-40.40s%s%s\n"
      ,IREC,id,fmt[bdio_get_rfmt(fh)],printlen(bdio_get_rlen(fh)),bdio_get_ruinfo(fh),str,
      lrec[(int)fh->rlongrec],RSET);
      print_record_meta(id, fh);
      printf("%s-------------------------------------------------------------------------------%s\n",METC,RSET);
   }
   if( data_flag && dindex==id )
   {
      if(!raw_flag)
      {
         for( i=0; i<rd2/4; i++)
         {
            printf("%" PRIi32 "\n",dat[i]);
         }
         free(dat);
      }else
      {
         fwrite(dat,1,rd2,stdout);
      }
   }

   if( (!meta_flag) && (!data_flag) )
   {
      if( bdio_get_rlen(fh)<rd )
         rd=bdio_get_rlen(fh);
      rd=bdio_read_int32(d,rd,fh);
      for (i=0; i<13*12+1; i++)
         str[i] = ' ';
      str[0]='\0';
      for( i=0; i<rd/4; i++)
      {
         sprintf(tmpstr,"%i ",d[i]);
         if( strlen(tmpstr)+strlen(str)<=40 )
            strcat(str,tmpstr);
      }
      printf("%s%-6li record %s %s byte %2i %-40.40s%s%s\n"
      ,IREC,id,fmt[bdio_get_rfmt(fh)],printlen(bdio_get_rlen(fh)),bdio_get_ruinfo(fh),str,
      lrec[(int)fh->rlongrec],RSET);
   }
}

void print_record_int64(long id, BDIO *fh)
{
   int64_t d[13];
   char str[13*21+1];
   char tmpstr[21+1];
   int i;
   size_t rd = 13*8,rd2;
   int64_t *dat;

   if( data_flag && dindex==id)
   {
      rd2 = bdio_get_rlen(fh);
      dat = (int64_t*) malloc(rd2);
      bdio_read_int64(dat,rd2,fh);
   }
   if( meta_flag && mindex==id)
   {
      printf("%s-------------------------------------------------------------------------------%s\n",METC,RSET);
      if( bdio_get_rlen(fh)<rd )
         rd=bdio_get_rlen(fh);
      if( data_flag && dindex==id )
         memcpy(d,dat,rd);
      else
         rd=bdio_read_int64(d,rd,fh);
      for (i=0; i<13*12+1; i++)
         str[i] = ' ';
      str[0]='\0';
      for( i=0; i<rd/8; i++)
      {
         sprintf(tmpstr,"%" PRIu64 " ",d[i]);
         if( strlen(tmpstr)+strlen(str)<=40 )
            strcat(str,tmpstr);
      }
      printf("%s%-6li record %s %s byte %2i %-40.40s%s%s\n"
      ,IREC,id,fmt[bdio_get_rfmt(fh)],printlen(bdio_get_rlen(fh)),bdio_get_ruinfo(fh),str,
      lrec[(int)fh->rlongrec],RSET);
      print_record_meta(id, fh);
      printf("%s-------------------------------------------------------------------------------%s\n",METC,RSET);
   }
   if( data_flag && dindex==id )
   {
      if(!raw_flag)
      {
         for( i=0; i<rd2/8; i++)
         {
            printf("%" PRIi64 "\n",dat[i]);
         }
      }else
      {
         fwrite(dat,1,rd2,stdout);
      }
      free(dat);
   }

   if( (!meta_flag) && (!data_flag) )
   {
      if( bdio_get_rlen(fh)<rd )
         rd=bdio_get_rlen(fh);
      rd=bdio_read_int64(d,rd,fh);
      for (i=0; i<13*21+1; i++)
         str[i] = ' ';
      str[0]='\0';
      for( i=0; i<rd/8; i++)
      {
         sprintf(tmpstr,"%" PRIi64 " ",d[i]);
         if( strlen(tmpstr)+strlen(str)<=40 )
            strcat(str,tmpstr);
      }
      printf("%s%-6li record %s %s byte %2i %-40.40s%s%s\n"
      ,IREC,id,fmt[bdio_get_rfmt(fh)],printlen(bdio_get_rlen(fh)),bdio_get_ruinfo(fh),str,
      lrec[(int)fh->rlongrec],RSET);
   }
}

void print_record_f32(long id, BDIO *fh)
{
   float d[6];
   char str[6*21+1];
   char tmpstr[21+1];
   int i, rd = 6*4;
   float *dat;
   int rd2;

   if( data_flag && dindex==id)
   {
      rd2 = bdio_get_rlen(fh);
      dat = (float*) malloc(rd2);
      bdio_read_f32(dat,rd2,fh);
   }
   if( meta_flag && mindex==id)
   {
      printf("%s-------------------------------------------------------------------------------%s\n",METC,RSET);
      if( bdio_get_rlen(fh)<rd )
         rd=bdio_get_rlen(fh);
      if( data_flag && dindex==id )
         memcpy(d,dat,rd);
      else
         rd=bdio_read_f32(d,rd,fh);
      for (i=0; i<6*21+1; i++)
         str[i] = ' ';
      str[0]='\0';
      for( i=0; i<rd/4; i++)
      {
         sprintf(tmpstr,"%e ",d[i]);
         if( strlen(tmpstr)+strlen(str)<=40 )
            strcat(str,tmpstr);
      }
      printf("%s%-6li record %s %s byte %2i %-40.40s%s%s\n"
      ,FREC,id,fmt[bdio_get_rfmt(fh)],printlen(bdio_get_rlen(fh)),bdio_get_ruinfo(fh),str,
      lrec[(int)fh->rlongrec],RSET);
      print_record_meta(id, fh);
      printf("%s-------------------------------------------------------------------------------%s\n",METC,RSET);
   }
   if( data_flag && dindex==id )
   {
      if(!raw_flag)
      {
         for( i=0; i<rd2/4; i++)
         {
            printf("%e\n",dat[i]);
         }
      }else
      {
         fwrite(dat,1,rd2,stdout);
      }
      free(dat);
   }

   if( (!meta_flag) && (!data_flag) )
   {
      if( bdio_get_rlen(fh)<rd )
         rd=bdio_get_rlen(fh);
      rd=bdio_read_f32(d,rd,fh);
      for (i=0; i<6*21+1; i++)
         str[i] = ' ';
      str[0]='\0';
      for( i=0; i<rd/4; i++)
      {
         sprintf(tmpstr,"%e ",d[i]);
         if( strlen(tmpstr)+strlen(str)<=40 )
            strcat(str,tmpstr);
      }
      printf("%s%-6li record %s %s byte %2i %-40.40s%s%s\n"
      ,FREC,id,fmt[bdio_get_rfmt(fh)],printlen(bdio_get_rlen(fh)),bdio_get_ruinfo(fh),str,
      lrec[(int)fh->rlongrec],RSET);
   }
}

void print_record_f64(long id, BDIO *fh)
{
   double d[6];
   char str[6*21+1];
   char tmpstr[21+1];
   int i, rd = 6*8;
   double *dat;
   int rd2;

   if( data_flag && dindex==id)
   {
      rd2 = bdio_get_rlen(fh);
      dat = (double*) malloc(rd2);
      bdio_read_f64(dat,rd2,fh);
   }
   if( meta_flag && mindex==id)
   {
      printf("%s-------------------------------------------------------------------------------%s\n",METC,RSET);
      if( bdio_get_rlen(fh)<rd )
         rd=bdio_get_rlen(fh);
      if( data_flag && dindex==id )
         memcpy(d,dat,rd);
      else
         rd=bdio_read_f64(d,rd,fh);
      for (i=0; i<6*21+1; i++)
         str[i] = ' ';
      str[0]='\0';
      for( i=0; i<rd/8; i++)
      {
         sprintf(tmpstr,"%e ",d[i]);
         if( strlen(tmpstr)+strlen(str)<=40 )
            strcat(str,tmpstr);
      }
      printf("%s%-6li record %s %s byte %2i %-40.40s%s%s\n"
      ,FREC,id,fmt[bdio_get_rfmt(fh)],printlen(bdio_get_rlen(fh)),bdio_get_ruinfo(fh),str,
      lrec[(int)fh->rlongrec],RSET);
      print_record_meta(id, fh);
      printf("%s-------------------------------------------------------------------------------%s\n",METC,RSET);
   }
   if( data_flag && dindex==id )
   {
      if(!raw_flag)
      {
         for( i=0; i<rd2/8; i++)
         {
            printf("%0.15e\n",dat[i]);
         }
      }else
      {
         fwrite(dat,1,rd2,stdout);
      }
      free(dat);
   }
   if( (!meta_flag) && (!data_flag) )
   {
      if( bdio_get_rlen(fh)<rd )
         rd=bdio_get_rlen(fh);
      rd=bdio_read_f64(d,rd,fh);
      for (i=0; i<6*21+1; i++)
         str[i] = ' ';
      str[0]='\0';
      for( i=0; i<rd/8; i++)
      {
         sprintf(tmpstr,"%e ",d[i]);
         if( strlen(tmpstr)+strlen(str)<=40 )
            strcat(str,tmpstr);
      }
      printf("%s%-6li record %s %s byte %2i %-40.40s%s%s\n"
      ,FREC,id,fmt[bdio_get_rfmt(fh)],printlen(bdio_get_rlen(fh)),bdio_get_ruinfo(fh),str,
      lrec[(int)fh->rlongrec],RSET);
   }
}

void print_record_ascii(long id, BDIO *fh)
{
   unsigned char str[41];
   int rd = 39;
   char *dat;
   int rd2;

   if( data_flag && dindex==id)
   {
      rd2 = bdio_get_rlen(fh);
      dat = malloc(rd2);
      bdio_read(dat,rd2,fh);
   }
   if( meta_flag && mindex==id)
   {
      printf("%s-------------------------------------------------------------------------------%s\n",METC,RSET);
      if( bdio_get_rlen(fh)<rd )
         rd=bdio_get_rlen(fh);
      if( data_flag && dindex==id )
         memcpy(str,dat,rd);
      else
         rd=bdio_read(str,rd,fh);
      str[rd]=' ';
      str[rd+1]='\0';
      strcleanup(str);
      printf("%s%-6li record %s %s byte %2i %-40.40s%s%s\n"
      ,AREC,id,fmt[bdio_get_rfmt(fh)],printlen(bdio_get_rlen(fh)),bdio_get_ruinfo(fh),str,
      lrec[(int)fh->rlongrec],RSET);
      print_record_meta(id, fh);
      printf("%s-------------------------------------------------------------------------------%s\n",METC,RSET);
   }
   if( data_flag && dindex==id )
   {
      if(!raw_flag)
      {
         strcleanup2(dat,rd2);
         fwrite(dat,1,rd2,stdout);
      }else
      {
         fwrite(dat,1,rd2,stdout);
      }
      
      free(dat);
   }

   if( (!meta_flag) && (!data_flag) )
   {
      if( bdio_get_rlen(fh)<rd )
         rd=bdio_get_rlen(fh);
      rd=bdio_read(str,rd,fh);
      str[rd]=' ';
      str[rd+1]='\0';
      strcleanup(str);
      printf("%s%-6li record %s %s byte %2i %-40.40s%s%s\n"
      ,AREC,id,fmt[bdio_get_rfmt(fh)],printlen(bdio_get_rlen(fh)),bdio_get_ruinfo(fh),str,
      lrec[(int)fh->rlongrec],RSET);
   }
}

void print_record_exe(long id, BDIO *fh)
{
   unsigned char str[41];
   int rd = 39;
   char *dat;
   int rd2;

   if( data_flag && dindex==id)
   {
      rd2 = bdio_get_rlen(fh);
      dat = malloc(rd2);
      bdio_read(dat,rd2,fh);
   }
   if( meta_flag && mindex==id)
   {
      printf("%s-------------------------------------------------------------------------------%s\n",METC,RSET);
      if( bdio_get_rlen(fh)<rd )
         rd=bdio_get_rlen(fh);
      if( data_flag && dindex==id )
         memcpy(str,dat,rd);
      else
         rd=bdio_read(str,rd,fh);
      str[rd]=' ';
      str[rd+1]='\0';
      strcleanup(str);
      printf("%s%-6li record %s %s byte %2i %-40.40s%s%s\n"
      ,EREC,id,fmt[bdio_get_rfmt(fh)],printlen(bdio_get_rlen(fh)),bdio_get_ruinfo(fh),str,
      lrec[(int)fh->rlongrec],RSET);
      print_record_meta(id, fh);
      printf("%s-------------------------------------------------------------------------------%s\n",METC,RSET);
   }
   if( data_flag && dindex==id )
   {
      fwrite(dat,1,rd2,stdout);
      free(dat);
   }

   if( (!meta_flag) && (!data_flag) )
   {
      if( bdio_get_rlen(fh)<rd )
         rd=bdio_get_rlen(fh);
      rd=bdio_read(str,rd,fh);
      str[rd]=' ';
      str[rd+1]='\0';
      strcleanup(str);
      printf("%s%-6li record %s %s byte %2i %-40.40s%s%s\n"
      ,EREC,id,fmt[bdio_get_rfmt(fh)],printlen(bdio_get_rlen(fh)),bdio_get_ruinfo(fh),str,
      lrec[(int)fh->rlongrec],RSET);
   }
}

void print_record_xml(long id, BDIO *fh)
{
   unsigned char str[41];
   int rd = 39;
   char *dat;
   int rd2;

   if( data_flag && dindex==id )
   {
      rd2 = bdio_get_rlen(fh);
      dat = malloc(rd2);
      bdio_read(dat,rd2,fh);
   }
   if( meta_flag && mindex==id)
   {
      printf("%s-------------------------------------------------------------------------------%s\n",METC,RSET);
      if( bdio_get_rlen(fh)<rd )
         rd=bdio_get_rlen(fh);
      if( data_flag && dindex==id )
         memcpy(str,dat,rd);
      else
         rd=bdio_read(str,rd,fh);
      str[rd]=' ';
      str[rd+1]='\0';
      strcleanup(str);
      printf("%s%-6li record %s %s byte %2i %-40.40s%s%s\n"
      ,XREC,id,fmt[bdio_get_rfmt(fh)],printlen(bdio_get_rlen(fh)),bdio_get_ruinfo(fh),str,
      lrec[(int)fh->rlongrec],RSET);
      print_record_meta(id, fh);
      printf("%s-------------------------------------------------------------------------------%s\n",METC,RSET);
   }
   if( data_flag && dindex==id )
   {
      fwrite(dat,1,rd2,stdout);
      free(dat);
   }

   if( (!meta_flag) && (!data_flag) )
   {   
      if( bdio_get_rlen(fh)<rd )
         rd=bdio_get_rlen(fh);
      rd=bdio_read(str,rd,fh);
      str[rd]=' ';
      str[rd+1]='\0';
      strcleanup(str);
      printf("%s%-6li record %s %s byte %2i %-40.40s%s%s\n"
      ,XREC,id,fmt[bdio_get_rfmt(fh)],printlen(bdio_get_rlen(fh)),bdio_get_ruinfo(fh),str,
      lrec[(int)fh->rlongrec],RSET);
   }
}


void print_record_bin(long id, BDIO *fh)
{
   char str[41];
   char tmpstr[4];
   unsigned char d[13];
   int i,rd = 13;
   char *dat;
   int rd2;
   unsigned char digest[16];
   int is_hash = bdio_is_hash_record(digest,fh);

   if( data_flag && dindex==id )
   {
      rd2 = bdio_get_rlen(fh);
      dat = malloc(rd2);
      bdio_read(dat,rd2,fh);
   }
   if( meta_flag && mindex==id )
   {
      printf("%s-------------------------------------------------------------------------------%s\n",METC,RSET);
      if( bdio_get_rlen(fh)<rd )
         rd=bdio_get_rlen(fh);
      if( data_flag && dindex==id )
         memcpy(str,dat,rd);
      else
         rd=bdio_read(str,rd,fh);
      for (i=0; i<40; i++)
         str[i] = ' ';
      str[0]='\0';
      for( i=0; i<rd; i++)
      {
         sprintf(tmpstr,"%02X ", d[i]);
         if( strlen(tmpstr)+strlen(str)<=40 )
            strcat(str,tmpstr);
      }
      printf("%s%-6li record %s %s byte %2i %-40.40s%s%s\n"
      ,BREC,id,fmt[bdio_get_rfmt(fh)],printlen(bdio_get_rlen(fh)),bdio_get_ruinfo(fh),str,
      lrec[(int)fh->rlongrec],RSET);
      print_record_meta(id, fh);
      printf("%s-------------------------------------------------------------------------------%s\n",METC,RSET);
   }
   if( data_flag && dindex==id )
   {
      if(!raw_flag)
      {
         for( i=0; i<rd2; i++)
         {
            printf("%02X", (unsigned char) dat[i]);
         }
      }else
      {
         fwrite(dat,1,rd2,stdout);
      }
      free(dat);
   }

   if( (!meta_flag) && (!data_flag) )
   {
      if(!is_hash)
      {
         if( bdio_get_rlen(fh)<rd )
            rd=bdio_get_rlen(fh);
         rd=bdio_read(d,rd,fh);
         for (i=0; i<40; i++)
            str[i] = ' ';
         str[0]='\0';
         for( i=0; i<rd; i++)
         {
            sprintf(tmpstr,"%02hX ",d[i]);
            if( strlen(tmpstr)+strlen(str)<=40 )
               strcat(str,tmpstr);
         }
         printf("%s%-6li record %s %s byte %2i %-40.40s%s%s\n"
         ,BREC,id,fmt[bdio_get_rfmt(fh)],printlen(bdio_get_rlen(fh)),bdio_get_ruinfo(fh),str,
         lrec[(int)fh->rlongrec],RSET);
      }else
      {
         for (i=0; i<40; i++)
            str[i] = ' ';
         str[0]='\0';
         for( i=0; i<16; i++)
         {
            sprintf(tmpstr,"%02hX",digest[i]);
            strcat(str,tmpstr);
         }
         printf("%s%-6li record MD5-h  %s byte %2i %-40.40s%s%s\n",CREC,id,printlen(bdio_get_rlen(fh)),bdio_get_ruinfo(fh),str,
         lrec[(int)fh->rlongrec],RSET);
      }
   }
}

void printhelp()
{
   printf("\nusage:\n");
   printf("   lsbdio [options] file\n\n");
   printf("   without options a listing of the contents of file"
               " is printed.\n\n");
   printf("   -h, --help       print this help message\n");
   printf("   -v, --version    print the program version\n");
   printf("   -m k, --meta=k   print the meta-information of the header\n");
   printf("                    or record with index k\n");
   printf("   -d k, --data=k   print the data content of the record with\n");
   printf("                    index k. If the item with index k is a header,\n");
   printf("                    the same output as with -m is printed.\n");
   printf("   -c k, --color=k  if k>0 the output is printed in color (default)\n");
   printf("                    otherwise no ANSI colors are used\n");
   printf("   -r, --raw        Affects how --data works. If this flag is present\n");
   printf("                    the content of records is printed in binary, otherwise\n");
   printf("                    (default) pretty printing is enabled.\n");
   
}

void printversion()
{
   printf("\nlsbdio version %s\n\n",LSBDIO_VERSION);
}

int main(int argc, char *argv[])
{
   BDIO *fh;
   int i;
   long id=0;
   long pnh=0,nh=0;
   int c;
   int col=1;
   char *endptr;

   int option_index = 0;
   static struct option long_options[] =
   {
      {"data",    1, NULL, 'd'},
      {"meta",    1, NULL, 'm'},
      {"color", 1, NULL, 'c'},
      {"help",    0, NULL, 'h'},
      {"version", 0, NULL, 'v'},
      {"raw", 0, NULL, 'r'},
      {NULL,      0, NULL, 0}
   };

   /* parse options */
   do
   {
      /* getopt_long stores the option index here.   */
      c = getopt_long (argc, argv, "d:m:c:hvr",
             long_options, &option_index);

      switch (c)
      {
         case 'v':
            printversion();
            exit(EXIT_SUCCESS);
         case 'h':
            printhelp();
            exit(EXIT_SUCCESS);
         case 'c':
            errno=0;
            col = strtol(optarg,&endptr,10);
            if( (errno != 0) || (endptr == optarg))
            {
               fprintf(stderr,"Could not convert %s into an integer.\n",optarg);
               exit(EXIT_FAILURE);
            }
            break;
         case 'd':
            errno=0;
            dindex = strtol(optarg,&endptr,10);
            if( (errno != 0) || (endptr == optarg))
            {
               fprintf(stderr,"Could not convert %s into an integer.\n",optarg);
               exit(EXIT_FAILURE);
            }
            data_flag=1;
            break;
         case 'm':
            errno=0;
            mindex = strtol(optarg,&endptr,10);
            if( (errno != 0) || (endptr == optarg))
            {
               fprintf(stderr,"Could not convert %s into an integer.\n",optarg);
               exit(EXIT_FAILURE);
            }
            meta_flag=1;
            break;
         case '?':
            printhelp();
            exit(EXIT_FAILURE);
            break;
         case 'r':
            raw_flag=1;
            break;
         case -1: break;
         default:
            exit(EXIT_FAILURE);
      }
   }while(c != -1 );

   setcolor(col);

   /* set error stream to stderr and turn on verbose mode */
   bdio_set_dflt_msg(stderr);
   bdio_set_dflt_verbose(1);

   if( optind >= argc)
   {
      fprintf(stderr,"BDIO file-name missing.");
      printhelp();
      exit(EXIT_FAILURE);
   }

   if((fh = bdio_open( argv[optind], "r", NULL ))==NULL)
   {
      exit(EXIT_FAILURE);
   }

   if (!data_flag && !meta_flag)
      printf("\nID     record type       size   uinf starts with                           long\n");
   /* seek all the records */
   while(bdio_seek_record(fh)!=EOF)
   {
      /*
      if(bdio_seek_record(fh)==EOF)
      {
         printf("\n");
         exit(EXIT_FAILURE);
      }
      */
      if( (nh=bdio_get_hcnt(fh)) > pnh )
      {
         for(i=0; i< nh-pnh-1; i++)
         {
            print_empty_header(id);
            id++;
         }
         print_header(id,fh);
         id++;
         pnh=bdio_get_hcnt(fh);
      }
      if( bdio_is_in_record(fh) )
      {
         if( bdio_get_rfmt(fh)==BDIO_BIN_F64BE || bdio_get_rfmt(fh)==BDIO_BIN_F64LE )
         {
            print_record_f64(id,fh);
            id++;
         }
         if( bdio_get_rfmt(fh)==BDIO_BIN_F32BE || bdio_get_rfmt(fh)==BDIO_BIN_F32LE )
         {
            print_record_f32(id,fh);
            id++;
         }
         if( bdio_get_rfmt(fh)==BDIO_BIN_INT32BE || bdio_get_rfmt(fh)==BDIO_BIN_INT32LE )
         {
            print_record_int32(id,fh);
            id++;
         }
         if( bdio_get_rfmt(fh)==BDIO_BIN_INT64BE || bdio_get_rfmt(fh)==BDIO_BIN_INT64LE )
         {
            print_record_int64(id,fh);
            id++;
         }
         if( bdio_get_rfmt(fh)==BDIO_ASC_GENERIC )
         {
            print_record_ascii(id,fh);
            id++;
         }
         if( bdio_get_rfmt(fh)==BDIO_ASC_EXEC )
         {
            print_record_exe(id,fh);
            id++;
         }
         if( bdio_get_rfmt(fh)==BDIO_ASC_XML )
         {
            print_record_xml(id,fh);
            id++;
         }
         if( bdio_get_rfmt(fh)==BDIO_BIN_GENERIC )
         {
            print_record_bin(id,fh);
            id++;
         }
      }
   }
   if( (!data_flag) && (!meta_flag) )
      printf("\n");
   bdio_close(fh);
   exit(EXIT_SUCCESS);
}
