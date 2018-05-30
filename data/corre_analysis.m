function [ ] = corre_analysis(y, y_bin )
n = size(y,1);
step = 0.01;
y_win = y(y_bin ==1);
y_lose = y(y_bin ==0);
n_win = size(y_win,1);
n_lose = size(y_lose,1);
mean(y_lose)
mean(y_win)
%[f_win,xi] = ksdensity(y_win,-2.5:step:2.5); 
[f_win,xi] = ksdensity(y_win,0:step:1); 
figure(1)
plot(xi,f_win,'r');
hold on
%[f_win,xi] = ksdensity(y_win,-2.5:step:2.5)
[f_lose,xi] = ksdensity(y_lose,0:step:1); 
plot(xi,f_lose,'b');
p_win = n_win*f_win./(n_win*f_win+n_lose*f_lose);
 S = [xi;p_win];
% S(2,S(1,:)<= -2.1) = 0;
% S(2,S(1,:)>= 2.1) = 1;
p_win = S(2,:);
figure(2)
plot(xi,p_win,'b');
hold on
p_win_mt = lsqisotonic(xi,p_win);
plot(xi,p_win_mt,'r');
end