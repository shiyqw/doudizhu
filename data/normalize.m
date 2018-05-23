function [ y_new ] = normalize( y,eps )
%UNTITLED2 此处显示有关此函数的摘要
%   此处显示详细说明

a = min(y);
b = max(y);
y_new = (y.*(y>=0)/b - y.*(y<0)/a +1)/2;
y_new = y_new *(1-eps) + 0.5*eps;

end

