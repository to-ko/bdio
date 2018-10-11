% bdio_seek_record.m
%
% res = bdio_seek_record(BDIO)
%
% res:    0 if successfull, -1 otherwise
%
% BDIO:   bdio file descriptor of an open BDIO file
% 

%
% Tomasz Korzec 2014
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


function res = bdio_seek_record(BDIO)
   global bdio_structs bdio_N bdio_valid;
   if isempty(bdio_N) || isempty(bdio_structs) || isempty(bdio_valid)
      fprintf('Not a valid BDIO file.\n');
      res = -1;
      return;
   end
   if length(bdio_valid)<BDIO || ~bdio_valid(BDIO)
      fprintf('Not a valid BDIO file.\n');
      res = -1;
      return;
   end
   
   b = bdio_structs{BDIO};
   if b.mode ~= 0
      fprintf('Error in bdio_seek_record. Not in read mode.\n');
      res = -1;
      return;
   end
   
   if (b.state == 2) || (b.state == 1)
      if( fseek(b.fp, b.rlen-b.ridx, 'cof')==-1 )
         fprintf('Error in bdio_seek_record. fseek failed.\n');
         b.state = 4;
         res = -1;
         return;
      end
      b.ridx = b.rlen;
   end

   
   % read the header of the following record
   [hdr,count] = fread( b.fp, 1, 'uint32=>uint32','l');
   if feof( b.fp )
      % clean EOF reached
      b.state = 3;
      res = 0;
      return;
   end
   if  count ~= 1 
      fprintf('Error in bdio_seek_record. fread failed.\n');
      b.state = 4;
      res = -1;
      return;
   end
   b.rstart = b.rstart+b.rlen;
   b.ridx=4;
   while ~bitand(hdr,hex2dec('00000001')) 
      % must be a header
      % scroll back 4 bytes and read header
      if( fseek(b.fp, -4, 'cof')==-1 )
         fprintf('Error in bdio_seek_record. fseek failed.\n');
         b.state = 4;
         res = -1;
         return;
      end
      b.ridx=0;
      bdio_structs{BDIO}=b;
      if( ~read_header(BDIO) )
         b.state = 4;
         res = -1;
         return;
      end
      b=bdio_structs{BDIO};
      % seek next record
      if( fseek(b.fp, b.rlen-b.ridx, 'cof')==-1 )
         fprintf('Error in bdio_seek_record. fseek failed.\n');
         b.state = 4;
         res = -1;
         return;
      end
      b.ridx=b.rlen;
      % read the header of the following record (version 2 in the notes)
      [hdr, count] = fread(b.fp, 1, 'uint32=>uint32','l');
      if feof( b.fp ) 
         % clean EOF reached
         b.state = 3;
         res = 0;
         return;
      end
      if count ~= 1
         fprintf('Error in bdio_seek_record. fread failed.\n');
         b.state = 4;
         res = -1;
         return;
      end
      b.rstart = b.rstart+b.rlen;
      b.ridx = 4;
   end
   % can only be a data record
   b.rcnt=b.rcnt+1;
   b.rlongrec = bitshift(bitand(hdr,hex2dec('00000008')),-3);
   if b.rlongrec
      % scroll back 4 bytes and read  64 bit header
      if( fseek(b.fp, -4, 'cof')==-1 )
         fprintf('Error in bdio_seek_record. fseek failed.\n');
         b.state = 4;
         res = -1;
         return;
      end
      [lhdr,count] = fread(b.fp, 1, 'uint64=>uint64', 'l');
      if feof( b.fp )
         fprintf('Error in bdio_seek_record. Unexpected EOF.\n');
         b.state = 4;
         res = -1;
         return;
      end
      if count ~= 1
         fprintf('Error in bdio_seek_record. fread failed.\n');
         b.state = 4;
         res = -1;
         return;
      end
      b.rfmt     =  double(bitshift( bitand(lhdr,hex2dec('00000000000000f0')),-4));
      b.ruinfo   =  double(bitshift( bitand(lhdr,hex2dec('0000000000000f00')),-8));
      b.rlen     =  bitshift( bitand(lhdr,hex2dec('fffffffffffff000')),-12) + 8;
      if b.rlen > uint64(hex2dec('fffffffffffff'))
         fprintf('Error in bdio_seek_record. Record too long for a double.\n');
         b.state = 4;
         res = -1;
         return;
      end
      b.ridx     = 8;
   else
      b.rfmt     =  double(bitshift(bitand(hdr,hex2dec('000000f0')),-4));
      b.ruinfo   =  double(bitshift(bitand(hdr,hex2dec('00000f00')),-8));
      b.rlen     =  double(bitshift(bitand(hdr,hex2dec('fffff000')),-12) + 4);
   end

   % find out whether data items will have 1, 4 or 8 bytes 
   b.rdsize=1;
   if   (b.rfmt==3) || (b.rfmt==2) ...
      ||(b.rfmt==7) || (b.rfmt==6) 
      b.rdsize=4;
   end
   if   (b.rfmt==5) || (b.rfmt==4) ...
      ||(b.rfmt==9) || (b.rfmt==8)
      b.rdsize=8;
   end
   b.state  = 2;
   bdio_structs{BDIO} = b;
   res = 0;
   return;
end


function success = read_header(BDIO)
   global bdio_structs bdio_N bdio_valid;
   
   if isempty(bdio_N) || isempty(bdio_structs) || isempty(bdio_valid)
      success = false;
      return;
   end
   if length(bdio_valid)<BDIO || ~bdio_valid(BDIO)
      success = false;
      return;
   end
   
   b = bdio_structs{BDIO};
   % read header up to length entry
   if b.ridx>0 
      fprintf('Error in read_header. Not at the beginning of header.\n');
      success = false;
      return;
   end
   [hdr,count] = fread(b.fp,2,'uint32=>uint32','l');
   
   if count ~= 2
      fprintf('Error in read_header. Unexpected end of file\n');
      success = false;
      return;
   end

   if hdr(1) ~= 2147209342
      fprintf('Error in read_header. Not a valid bdio file.\n');
      success = false;
      return;
   end

   b.hversion = bitshift(bitand(hdr(2), hex2dec('ffff0000')),-16);
   len        =          bitand(hdr(2), hex2dec('00000fff'));

   [hdr,count] = fread(b.fp, 3, 'uint32=>uint32', 'l');
   if count ~= 3
      fprintf('Error in read_header. Unexpected end of file\n');
      success = false;
      return;
   end
   b.dirinfo1 = bitshift(bitand(hdr(1),hex2dec('ffc00000')),-22);
   b.dirinfo2 =          bitand(hdr(1),hex2dec('003fffff'));
   b.hcdate   = hdr(2);
   b.hmdate   = hdr(3);
   
   [buf,count] = fread(b.fp, len-12, 'uint8=>uint8');
   if count ~= len-12
      fprintf('Error in read_header. Unexpected end of file\n');
      success = false;
      return;
   end
   b.hssize=len-12;
   
   buf = strsplit(char(buf).',char(0));
   if length(buf) ~= 5
      b.hcuser = '';
      b.hmuser = '';
      b.hchost = '';
      b.hmhost = '';
      b.hpinfo = '';   
   end
   b.hcuser = buf{1};
   b.hmuser = buf{2};
   b.hchost = buf{3};
   b.hmhost = buf{4};
   b.hpinfo = buf{5};

   % set the remaining  header fields 
   b.hcnt=b.hcnt+1;
   b.state = 1;
   b.hstart = b.rstart;
   b.rlen = len+8;
   b.ridx = len+8;
   b.rlongrec = 0;
   success = true;
   bdio_structs{bdio_N} = b;
   return;
end
