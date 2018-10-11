% bdio_get_hmuser.m
%
% muser = bdio_get_hmuser(BDIO)
%
% muser:  string with name of the user who modified the last header
%         last
%
% BDIO:   BDIO file descriptor (obtained from bdio_open)
%
%
% Tomasz Korzec 2014
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function muser = bdio_get_hmuser(BDIO)
   global bdio_structs bdio_N bdio_valid;
   if isempty(bdio_N) || isempty(bdio_structs) || isempty(bdio_valid)
      fprintf('Error in bdio_get_hmuser. Not a valid BDIO file.\n');
      rd = [];
      return;
   end
   if length(bdio_valid)<BDIO || ~bdio_valid(BDIO)
      fprintf('Error in bdio_get_hmuser. Not a valid BDIO file.\n');
      rd = [];
      return;
   end

   muser = bdio_structs{BDIO}.hmuser;
   return
end
