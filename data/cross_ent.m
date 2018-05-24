function [ h ] = cross_ent( y, y_hat )
%UNTITLED4 此处显示有关此函数的摘要
%   此处显示详细说明
eps = 1e-10;
h = -mean(y.*log(max(y_hat,eps))+ (1-y).*(log(max(1-y_hat,eps))));

end

