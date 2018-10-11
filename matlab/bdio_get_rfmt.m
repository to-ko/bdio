% bdio_get_rfmt.m
%
% fmt = bdio_get_rfmt(BDIO)
%
% fmt:    record format of current record
%
% BDIO:   BDIO file descriptor (obtained from bdio_open)
%

%
% Tomasz Korzec 2014
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function fmt = bdio_get_rfmt(BDIO)
   global bdio_structs bdio_N bdio_valid;
   if isempty(bdio_N) || isempty(bdio_structs) || isempty(bdio_valid)
      fprintf('Error in bdio_get_rfmt. Not a valid BDIO file.\n');
      fmt = [];
      return;
   end
   if length(bdio_valid)<BDIO || ~bdio_valid(BDIO)
      fprintf('Error in bdio_get_rfmt. Not a valid BDIO file.\n');
      fmt = [];
      return;
   end
   if bdio_structs{BDIO}.state ~= 2
      fprintf('Error in bdio_get_rfmt. Not in a record.\n');
      fmt = [];
      return;
   end
   fmt = bdio_structs{BDIO}.rfmt;
   return
end
