% bdio_get_hversion.m
%
% version = bdio_get_hversion(BDIO)
%
% version:  bdio version number of last header
%
% BDIO:   BDIO file descriptor (obtained from bdio_open)
%

%
% Tomasz Korzec 2014
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function version = bdio_get_hversion(BDIO)
   global bdio_structs bdio_N bdio_valid;
   if isempty(bdio_N) || isempty(bdio_structs) || isempty(bdio_valid)
      fprintf('Error in bdio_get_hversion. Not a valid BDIO file.\n');
      version = [];
      return;
   end
   if length(bdio_valid)<BDIO || ~bdio_valid(BDIO)
      fprintf('Error in bdio_get_hversion. Not a valid BDIO file.\n');
      version = [];
      return;
   end

   version = bdio_structs{BDIO}.hversion;
   return
end
