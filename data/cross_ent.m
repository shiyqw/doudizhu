function [ h ] = cross_ent( y, y_hat )
%UNTITLED4 �˴���ʾ�йش˺�����ժҪ
%   �˴���ʾ��ϸ˵��
eps = 1e-10;
h = -mean(y.*log(max(y_hat,eps))+ (1-y).*(log(max(1-y_hat,eps))));

end

