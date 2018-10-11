% bdio_get_hmdate_str.m
%
% mdate_str = bdio_get_hmdate_str(BDIO)
%
% mdate_str:  string with modification date of last header
%
% BDIO:   BDIO file descriptor (obtained from bdio_open)
%
%
% Tomasz Korzec 2014
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function mdate_str = bdio_get_hmdate_str(BDIO)
   global bdio_structs bdio_N bdio_valid;
   if isempty(bdio_N) || isempty(bdio_structs) || isempty(bdio_valid)
      fprintf('Error in bdio_get_hmdate_str. Not a valid BDIO file.\n');
      mdate_str = [];
      return;
   end
   if length(bdio_valid)<BDIO || ~bdio_valid(BDIO)
      fprintf('Error in bdio_get_hmdate_str. Not a valid BDIO file.\n');
      mdate_str = [];
      return;
   end

   mdate_str = datestr(double(bdio_structs{BDIO}.hmdate)/86400 + datenum(1970,1,1));
   return
end
