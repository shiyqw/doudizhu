% A=[importdata('train/1/xiajia.dat');
%     importdata('train/2/xiajia.dat');
%     importdata('train/3/xiajia.dat');
%     importdata('train/4/xiajia.dat');
%     importdata('train/5/xiajia.dat');
%     importdata('train/6/xiajia.dat');
%     importdata('train/7/xiajia.dat');
%     importdata('train/8/xiajia.dat')];
% A=[importdata('train/1/menban.dat');
%     importdata('train/2/menban.dat');
%     importdata('train/3/menban.dat');
%     importdata('train/4/menban.dat');
%     importdata('train/5/menban.dat');
%     importdata('train/6/menban.dat');
%     importdata('train/7/menban.dat');
%     importdata('train/8/menban.dat')];

A=[importdata('train/1/dizhu.dat');
    importdata('train/2/dizhu.dat');
    importdata('train/3/dizhu.dat');
    importdata('train/4/dizhu.dat');
    importdata('train/5/dizhu.dat');
    importdata('train/6/dizhu.dat');
    importdata('train/7/dizhu.dat');
    importdata('train/8/dizhu.dat')];
n = size(A,1);
d = size(A,2)-2;
step = 0.01;
y_bin = A(:,d+1);
y = A(:,d+2);
y_win = y(y_bin ==1);
y_lose = y(y_bin ==0);
n_win = size(y_win,1);
n_lose = size(y_lose,1);
[f_win,xi] = ksdensity(y_win,-2.5:step:2.5); 
figure(1)
plot(xi,f_win,'r');
hold on
[f_lose,xi] = ksdensity(y_lose,-2.5:step:2.5); 
plot(xi,f_lose,'b');
p_win = n_win*f_win./(n_win*f_win+n_lose*f_lose);
S = [xi;p_win];
S(2,S(1,:)<= -2.1) = 0;
S(2,S(1,:)>= 2.1) = 1;
p_win = S(2,:);
figure(2)
plot(xi,p_win,'b');
hold on
p_win_mt = lsqisotonic(xi,p_win)
plot(xi,p_win_mt,'r');
