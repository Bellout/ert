function [data1 , data2] = ins_plot2(prefix)

%function [d_enkf , d_eclipse ] = ins_plot2(prefix)
%
%This function is quite similar to the ins_plot, function
%and the documentation there will not be repeated. The difference
%is that it is designed to plot time series of variables which 
%have both an internal enkf state, and an external (transformed)
%state presented to ECLIPSE.
%
%The function call ins_plot() twice, appending ".enkf" and ".eclipse"
%to the prefix to generate the filenames for plotting. Both data sets
%are returned for subsequent analysis.

  [data1 , title1] = fread_thist(strcat(prefix , '.enkf'));
  [data2 , title2] = fread_thist(strcat(prefix , '.eclipse'));

  ins_plot__(data1 , title1, 1);
  ins_plot__(data2 , title2, 2);
