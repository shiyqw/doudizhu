function [ y_new ] = normalize( y,y_bin )
%UNTITLED2 此处显示有关此函数的摘要
%   此处显示详细说明
step = 0.01;
y_win = y(y_bin ==1);
y_lose = y(y_bin ==0);
n_win = size(y_win,1);
n_lose = size(y_lose,1);
[f_win,xi] = ksdensity(y_win,-2.5:step:2.5); 
[f_lose,xi] = ksdensity(y_lose,-2.5:step:2.5); 
p_win = n_win*f_win./(n_win*f_win+n_lose*f_lose);
S = [xi;p_win];
S(2,S(1,:)<= -2.1) = 0;
S(2,S(1,:)>= 2.1) = 1;
p_win = S(2,:);
p_win = lsqisotonic(xi,p_win);
n = size(y,1);
y_new = zeros(n,1);
for i=1:n
    k = round(y(i)*100) + 250;
    y_new(i) = p_win(k);
end

end

