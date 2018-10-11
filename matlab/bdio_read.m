% bdio_read.m
%
% rd = bdio_read(nb, BDIO)
%
% rd:     vector of uint8 values that were read from the BDIO file
%
% nb:     number of bytes to read
% BDIO:   BDIO file descriptor (obtained from bdio_open)
% 

%
% Tomasz Korzec 2014
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function rd = bdio_read(nb,BDIO)
   global bdio_structs bdio_N bdio_valid;
   if isempty(bdio_N) || isempty(bdio_structs) || isempty(bdio_valid)
      fprintf('Not a valid BDIO file.\n');
      rd = [];
      return;
   end
   if length(bdio_valid)<BDIO || ~bdio_valid(BDIO)
      fprintf('Not a valid BDIO file.\n');
      rd = [];
      return;
   end
   
   b = bdio_structs{BDIO};
   if b.state ~= 2
      fprintf('Error in bdio_read. No record seeked.\n');
      rd = [];
      return;
   end
   if b.mode ~= 0
      fprintf('Error in bdio_read. Not in read mode.\n');
      rd=[];
      return;
   end

   if mod(nb,b.rdsize) ~= 0
      fprintf('Error in bdio_read. nb is not multiple of data size.\n');
      rd=[];
      return;
   end

   if nb > (b.rlen-b.ridx)
      fprintf('Error in bdio_read. nb is larger than remaining data in the record.\n');
      rd = [];
      return;
   end
   
   [rd, count] = fread(b.fp, nb, 'uint8=>uint8');
   if count<nb
      if feof( b.fp )
         fprintf('Error in bdio_read. Unexpected EOF.\n');
      else
         fprintf('Error in bdio_read. fread failed.\n');
      end
   end
   % update b 
   b.ridx = b.ridx+count;
   bdio_structs{BDIO} = b;
   return;
end
