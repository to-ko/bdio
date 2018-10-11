% lsbdio.m
%
% lsbdio(file)
%
% lists the contents of a bdio file
% (wrapper to lsbdio from bdio/tools/)

%
% Tomasz Korzec 2014
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function lsbdio(file)
%TODO disable color if not supported by terminal / desktop
system(sprintf('./lsbdio %s',file));
end
