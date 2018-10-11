% bdio_get_ruinfo.m
%
% uinfo = bdio_get_ruinfo(BDIO)
%
% uinfo:  user info of current record
%
% BDIO:   BDIO file descriptor (obtained from bdio_open)
%

%
% Tomasz Korzec 2014
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function uinfo = bdio_get_ruinfo(BDIO)
   global bdio_structs bdio_N bdio_valid;
   if isempty(bdio_N) || isempty(bdio_structs) || isempty(bdio_valid)
      fprintf('Error in bdio_get_ruinfo. Not a valid BDIO file.\n');
      uinfo = [];
      return;
   end
   if length(bdio_valid)<BDIO || ~bdio_valid(BDIO)
      fprintf('Error in bdio_get_ruinfo. Not a valid BDIO file.\n');
      uinfo = [];
      return;
   end
   if bdio_structs{BDIO}.state ~= 2
      fprintf('Error in bdio_get_ruinfo. Not in a record.\n');
      uinfo = [];
      return;
   end
   uinfo = bdio_structs{BDIO}.ruinfo;
   return
end
