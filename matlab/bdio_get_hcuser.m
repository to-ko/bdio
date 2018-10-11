% bdio_get_hcuser.m
%
% cuser = bdio_get_hcuser(BDIO)
%
% cuser:  string with name of the user who created the last header
%
% BDIO:   BDIO file descriptor (obtained from bdio_open)
%

%
% Tomasz Korzec 2014
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function cuser = bdio_get_hcuser(BDIO)
   global bdio_structs bdio_N bdio_valid;
   if isempty(bdio_N) || isempty(bdio_structs) || isempty(bdio_valid)
      fprintf('Error in bdio_get_hcuser. Not a valid BDIO file.\n');
      cuser = [];
      return;
   end
   if length(bdio_valid)<BDIO || ~bdio_valid(BDIO)
      fprintf('Error in bdio_get_hcuser. Not a valid BDIO file.\n');
      cuser = [];
      return;
   end

   cuser = bdio_structs{BDIO}.hcuser;
   return
end
