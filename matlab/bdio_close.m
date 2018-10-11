% bdio_close.m
%
% res = bdio_close(BDIO)
%
% res:    0 if successfull, -1 otherwise
%
% BDIO:   bdio file descriptor of an open BDIO file
% 

%
% Tomasz Korzec 2014
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function res = bdio_close(BDIO)
   global bdio_structs bdio_N bdio_valid;
   if isempty(bdio_N) || isempty(bdio_structs) || isempty(bdio_valid)
      fprintf('Not a valid BDIO file.\n');
      res = -1;
      return;
   end
   if length(bdio_valid)<BDIO || ~bdio_valid(BDIO)
      fprintf('Not a valid BDIO file.\n');
      res = -1;
      return;
   end
   
   if ~isfield(bdio_structs{BDIO},'fp')
      fprintf('Not a valid BDIO file.\n');
      res=-1;
      return
   end
   if bdio_structs{BDIO}.fp == -1
      fprintf('Not an open BDIO file.\n');
      res=-1;
      return
   end
   %TODO: handle w and a modes
   fclose(bdio_structs{BDIO}.fp);
   bdio_structs{BDIO} = [];
   bdio_valid(BDIO) = false;
   res = 0;
end
