% bdio_get_hcdate_str.m
%
% cdate_str = bdio_get_hcdate_str(BDIO)
%
% cdate_str:  string with creation date of last header
%
% BDIO:   BDIO file descriptor (obtained from bdio_open)
%

%
% Tomasz Korzec 2014
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function cdate_str = bdio_get_hcdate_str(BDIO)
   global bdio_structs bdio_N bdio_valid;
   if isempty(bdio_N) || isempty(bdio_structs) || isempty(bdio_valid)
      fprintf('Error in bdio_get_hcdate_str. Not a valid BDIO file.\n');
      cdate_str = [];
      return;
   end
   if length(bdio_valid)<BDIO || ~bdio_valid(BDIO)
      fprintf('Error in bdio_get_hcdate_str. Not a valid BDIO file.\n');
      cdate_str = [];
      return;
   end

   cdate_str = datestr(double(bdio_structs{BDIO}.hcdate)/86400 + datenum(1970,1,1));
   return
end
