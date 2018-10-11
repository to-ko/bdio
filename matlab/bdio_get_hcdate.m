% bdio_get_hcdate.m
%
% cdate = bdio_get_hcdate(BDIO)
%
% cdate:  linux time stamp of creation date
%
% BDIO:   BDIO file descriptor (obtained from bdio_open)
%

%
% Tomasz Korzec 2014
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function cdate = bdio_get_hcdate(BDIO)
   global bdio_structs bdio_N bdio_valid;
   if isempty(bdio_N) || isempty(bdio_structs) || isempty(bdio_valid)
      fprintf('Error in bdio_get_hcdate. Not a valid BDIO file.\n');
      cdate = [];
      return;
   end
   if length(bdio_valid)<BDIO || ~bdio_valid(BDIO)
      fprintf('Error in bdio_get_hcdate. Not a valid BDIO file.\n');
      cdate = [];
      return;
   end

   cdate = bdio_structs{BDIO}.hcdate;
   return
end
