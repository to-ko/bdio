% bdio_get_hpinfo.m
%
% pinfo = bdio_get_hpinfo(BDIO)
%
% pinfo:  string with the protocol info of the last header
%
% BDIO:   BDIO file descriptor (obtained from bdio_open)
%

%
% Tomasz Korzec 2014
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function pinfo = bdio_get_hpinfo(BDIO)
   global bdio_structs bdio_N bdio_valid;
   if isempty(bdio_N) || isempty(bdio_structs) || isempty(bdio_valid)
      fprintf('Error in bdio_get_hpinfo. Not a valid BDIO file.\n');
      rd = [];
      return;
   end
   if length(bdio_valid)<BDIO || ~bdio_valid(BDIO)
      fprintf('Error in bdio_get_hpinfo. Not a valid BDIO file.\n');
      rd = [];
      return;
   end

   pinfo = bdio_structs{BDIO}.hpinfo;
   return
end
