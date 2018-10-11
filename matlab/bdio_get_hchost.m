% bdio_get_hchost.m
%
% chost = bdio_get_hchost(BDIO)
%
% chost:  string with name of the host on which the last header
%         was created
%
% BDIO:   BDIO file descriptor (obtained from bdio_open)
%

%
% Tomasz Korzec 2014
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function chost = bdio_get_hchost(BDIO)
   global bdio_structs bdio_N bdio_valid;
   if isempty(bdio_N) || isempty(bdio_structs) || isempty(bdio_valid)
      fprintf('Error in bdio_get_hchost. Not a valid BDIO file.\n');
      chost = [];
      return;
   end
   if length(bdio_valid)<BDIO || ~bdio_valid(BDIO)
      fprintf('Error in bdio_get_hchost. Not a valid BDIO file.\n');
      chost = [];
      return;
   end

   chost = bdio_structs{BDIO}.hchost;
   return
end
