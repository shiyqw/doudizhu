function [ y_new ] = normalize( y,eps )
%UNTITLED2 �˴���ʾ�йش˺�����ժҪ
%   �˴���ʾ��ϸ˵��

a = min(y);
b = max(y);
y_new = (y.*(y>=0)/b - y.*(y<0)/a +1)/2;
y_new = y_new *(1-eps) + 0.5*eps;

end

