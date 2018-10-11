% bdio_read_f64.m
%
% rd = bdio_read_f64(nb, BDIO)
%
% rd:     vector of double values that were read from the BDIO file
%
% nb:     number of bytes to read
% BDIO:   BDIO file descriptor (obtained from bdio_open)
% 

%
% Tomasz Korzec 2014
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function rd = bdio_read_f64(nb,BDIO)
   global bdio_structs bdio_N bdio_valid;
   if isempty(bdio_N) || isempty(bdio_structs) || isempty(bdio_valid)
      fprintf('Error in bdio_read_f64. Not a valid BDIO file.\n');
      rd = [];
      return;
   end
   if length(bdio_valid)<BDIO || ~bdio_valid(BDIO)
      fprintf('Error in bdio_read_f64. Not a valid BDIO file.\n');
      rd = [];
      return;
   end
   
   b = bdio_structs{BDIO};
   if b.state ~= 2
      fprintf('Error in bdio_read_f64. No record seeked.\n');
      rd = [];
      return;
   end
   if b.mode ~= 0
      fprintf('Error in bdio_read_f64. Not in read mode.\n');
      rd=[];
      return;
   end

   if mod(nb,b.rdsize) ~= 0
      fprintf('Error in bdio_read_f64. nb is not multiple of data size.\n');
      rd=[];
      return;
   end

   if nb > (b.rlen-b.ridx)
      fprintf('Error in bdio_read_f64. nb is larger than remaining data in the record.\n');
      rd = [];
      return;
   end
   
   switch b.rfmt
      case 8 % F64BE
         [rd, count] = fread(b.fp, nb/8, 'float64=>float64','b');
         count = count*8;
      case 9 % F64LE
         [rd, count] = fread(b.fp, nb/8, 'float64=>float64','l');
         count = count*8;
      case 0 % GENERIC BINARY
         [rd, count] = fread(b.fp, nb/8, 'float64=>float64');
         count = count*8;
      otherwise
         fprintf('Error in bdio_read_f64. Record has incompatible format.\n');
         rd=[];
         return;
   end
   
   if count<nb
      if feof( b.fp )
         fprintf('Error in bdio_read_f64. Unexpected EOF.\n');
      else
         fprintf('Error in bdio_read_f64. fread failed.\n');
      end
   end
   % update b 
   b.ridx = b.ridx+count;
   bdio_structs{BDIO} = b;
   return;
end
