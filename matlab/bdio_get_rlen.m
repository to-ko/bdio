% bdio_get_rlen.m
%
% len = bdio_get_rlen(BDIO)
%
% len:  length of data content of current record in bytes
%
% BDIO:   BDIO file descriptor (obtained from bdio_open)
%

%
% Tomasz Korzec 2014
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function len = bdio_get_rlen(BDIO)
   global bdio_structs bdio_N bdio_valid;
   if isempty(bdio_N) || isempty(bdio_structs) || isempty(bdio_valid)
      fprintf('Error in bdio_get_rlen. Not a valid BDIO file.\n');
      len = [];
      return;
   end
   if length(bdio_valid)<BDIO || ~bdio_valid(BDIO)
      fprintf('Error in bdio_get_rlen. Not a valid BDIO file.\n');
      len = [];
      return;
   end
   if bdio_structs{BDIO}.state ~= 2
      fprintf('Error in bdio_get_rlen. Not in a record.\n');
      len = 0;
      return;
   end
   if bdio_structs{BDIO}.rlongrec==1
      len = bdio_structs{BDIO}.rlen-8;
   else
      len = bdio_structs{BDIO}.rlen-4;
   end
   return
end
