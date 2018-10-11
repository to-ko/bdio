% bdio_get_rcnt.m
%
% cnt = bdio_get_rcnt(BDIO)
%
% cnt:  number of records encountered so far
%
% BDIO:   BDIO file descriptor (obtained from bdio_open)
%

%
% Tomasz Korzec 2014
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function cnt = bdio_get_rcnt(BDIO)
   global bdio_structs bdio_N bdio_valid;
   if isempty(bdio_N) || isempty(bdio_structs) || isempty(bdio_valid)
      fprintf('Error in bdio_get_rcnt. Not a valid BDIO file.\n');
      cnt = [];
      return;
   end
   if length(bdio_valid)<BDIO || ~bdio_valid(BDIO)
      fprintf('Error in bdio_get_rcnt. Not a valid BDIO file.\n');
      cnt = [];
      return;
   end
   cnt = bdio_structs{BDIO}.rcnt;
   return
end
