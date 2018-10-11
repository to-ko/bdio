% bdio_open.m
%
% BDIO = bdio_open(file, mode, protocol_info)
%
% BDIO:          bdio file descriptor
% 
% mode:          'r','a','w' for read, append or write mode.
% protocol_info: user supplied protocol_info to be written into the file
%                header, or compared with an existing file header.
%                protocol_info is optional in 'r' and 'a' modes.

%
% Tomasz Korzec 2014
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function BDIO = bdio_open(file, mode, protocol_info)
   global bdio_structs bdio_N bdio_valid;
   
   %TODO test inputs
   
   if isempty(bdio_N) || isempty(bdio_structs) || isempty(bdio_valid)
      bdio_N = 1;
      bdio_valid(1) = true;
      bdio_structs = cell(1);
      BDIO = 1;
   else
      bdio_N = bdio_N+1;
      bdio_valid(bdio_N) = true;
      BDIO = bdio_N;
      bdio_structs{BDIO} = [];
   end
   
   if nargin==2
      protocol_info = [];
   end
   if ~strcmp(mode,'r')
      fprintf('Only read mode supported so far.\n')
      BDIO = -1;
      return
   end
   [fid, message] = fopen(file,'r');
   if fid==-1
      fprintf('%s\n',message);
      BDIO = -1;
   end
   b.fp     = fid;
   b.mode   = 0;
   b.hcnt   = 0;
   b.rcnt   = 0;
   b.rstart = 0;
   b.rlen   = 0;
   b.ridx   = 0;
   b.ruinfo = [];
   b.rdsize = [];
   b.rfmt   = [];
   bdio_structs{BDIO}=b;
   if ~read_header(BDIO);
      BDIO = -1;
      return;
   end
   if ~isempty(protocol_info) && ~strcmp(protocol_info,bdio_structs{BDIO}.hpinfo)
      fprintf('WARNING! protocol_infos do not match.\n')
   end
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
   len        =   double(bitand(hdr(2), hex2dec('00000fff')));

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
   
   buf = strsplit_bdio(char(buf).',char(0));
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

function res = strsplit_bdio(str, sep)
   % strsplit does not exist in older octave/matlab versions, so 
   % here is a simple (and slow) implementation
   start=1;
   stop=0;
   n=0;
   for l=1:length(str)
      stop=stop+1;
      if str(l)==sep
         n=n+1;
         res{n} = str(start:stop-1);
         start=l+1;
         stop=l;
      end
   end
   if stop>=start
      res(n+1)=str(start:stop);
   end
end
