% bdio_get_hmdate.m
%
% mdate = bdio_get_hmdate(BDIO)
%
% mdate:  linux time stamp of modification date of last header
%
% BDIO:   BDIO file descriptor (obtained from bdio_open)
%

%
% Tomasz Korzec 2014
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function mdate = bdio_get_hmdate(BDIO)
   global bdio_structs bdio_N bdio_valid;
   if isempty(bdio_N) || isempty(bdio_structs) || isempty(bdio_valid)
      fprintf('Error in bdio_get_hmdate. Not a valid BDIO file.\n');
      mdate = [];
      return;
   end
   if length(bdio_valid)<BDIO || ~bdio_valid(BDIO)
      fprintf('Error in bdio_get_hmdate. Not a valid BDIO file.\n');
      mdate = [];
      return;
   end

   mdate = bdio_structs{BDIO}.hmdate;
   return
end
