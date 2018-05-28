eps = 0.01;
% A=[importdata('train/1/menban.dat');
%     importdata('train/2/menban.dat');
%     importdata('train/3/menban.dat');
%     importdata('train/4/menban.dat');
%     importdata('train/5/menban.dat');
%     importdata('train/6/menban.dat');
%     importdata('train/7/menban.dat');
%     importdata('train/8/menban.dat')];
% n = size(A,1);
% d = size(A,2)-2;
% X = A(:,1:d);
% y_bin = A(:,d+1);
% y = A(:,d+2);
% y = normalize(y,y_bin);
% X = make_feature(X);
% d = size(X,2);
% n_train = floor(n*0.8);
% X_train = X(1:n_train,1:d);
% y_bin_train = y_bin(1:n_train);
% y_train = y(1:n_train);
% X_test = X(n_train+1:n,1:d);
% y_bin_test = y_bin(n_train+1:n);
% y_test = y(n_train+1:n);
% n_test = size(X_test,1);
% csvwrite('y_train_menban.csv',y_train)
% csvwrite('x_train_menban.csv',X_train)
% csvwrite('y_test_menban.csv',y_test)
% csvwrite('x_test_menban.csv',X_test)
% csvwrite('y_bin_train_menban.csv',y_bin_train)
% csvwrite('y_bin_test_menban.csv',y_bin_test)
% 
% A=[importdata('train/1/xiajia.dat');
%     importdata('train/2/xiajia.dat');
%     importdata('train/3/xiajia.dat');
%     importdata('train/4/xiajia.dat');
%     importdata('train/5/xiajia.dat');
%     importdata('train/6/xiajia.dat');
%     importdata('train/7/xiajia.dat');
%     importdata('train/8/xiajia.dat')];
% n = size(A,1);
% d = size(A,2)-2;
% X = A(:,1:d);
% y_bin = A(:,d+1);
% y = A(:,d+2);
% y = normalize(y,y_bin);
% X = make_feature(X);
% d = size(X,2);
% n_train = floor(n*0.8);
% X_train = X(1:n_train,1:d);
% y_bin_train = y_bin(1:n_train);
% y_train = y(1:n_train);
% X_test = X(n_train+1:n,1:d);
% y_bin_test = y_bin(n_train+1:n);
% y_test = y(n_train+1:n);
% n_test = size(X_test,1);
% csvwrite('y_train_xiajia.csv',y_train)
% csvwrite('x_train_xiajia.csv',X_train)
% csvwrite('y_test_xiajia.csv',y_test)
% csvwrite('x_test_xiajia.csv',X_test)
% csvwrite('y_bin_train_xiajia.csv',y_bin_train)
% csvwrite('y_bin_test_xiajia.csv',y_bin_test)

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
X = A(:,1:d);
y_bin = A(:,d+1);
y = A(:,d+2);
y = normalize(y,y_bin);
X = make_feature(X);
d = size(X,2);
n_train = floor(n*0.8);
X_train = X(1:n_train,1:d);
y_bin_train = y_bin(1:n_train);
y_train = y(1:n_train);
X_test = X(n_train+1:n,1:d);
y_bin_test = y_bin(n_train+1:n);
y_test = y(n_train+1:n);
n_test = size(X_test,1);
csvwrite('y_train_dizhu.csv',y_train)
csvwrite('x_train_dizhu.csv',X_train)
csvwrite('y_test_dizhu.csv',y_test)
csvwrite('x_test_dizhu.csv',X_test)
csvwrite('y_bin_train_dizhu.csv',y_bin_train)
csvwrite('y_bin_test_dizhu.csv',y_bin_test)
