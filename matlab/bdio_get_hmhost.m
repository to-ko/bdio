% bdio_get_hmhost.m
%
% mhost = bdio_get_hmhost(BDIO)
%
% mhost:  string with name of the host on which the last header
%         was created
%
% BDIO:   BDIO file descriptor (obtained from bdio_open)
%

%
% Tomasz Korzec 2014
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function mhost = bdio_get_hmhost(BDIO)
   global bdio_structs bdio_N bdio_valid;
   if isempty(bdio_N) || isempty(bdio_structs) || isempty(bdio_valid)
      fprintf('Error in bdio_get_hmhost. Not a valid BDIO file.\n');
      rd = [];
      return;
   end
   if length(bdio_valid)<BDIO || ~bdio_valid(BDIO)
      fprintf('Error in bdio_get_hmhost. Not a valid BDIO file.\n');
      rd = [];
      return;
   end

   mhost = bdio_structs{BDIO}.hchost;
   return
end
