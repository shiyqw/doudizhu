%A=importdata('greedy/v2/dizhu.dat');
%A=importdata('greedy/v2/xiajia.dat');

% A=[importdata('train/1/dizhu.dat');
%     importdata('train/2/dizhu.dat');
%     importdata('train/3/dizhu.dat');
%     importdata('train/4/dizhu.dat');
%     importdata('train/5/dizhu.dat');
%     importdata('train/6/dizhu.dat');
%     importdata('train/7/dizhu.dat');
%     importdata('train/8/dizhu.dat')];

A=[importdata('train/1/xiajia.dat');
    importdata('train/2/xiajia.dat');
    importdata('train/3/xiajia.dat');
    importdata('train/4/xiajia.dat');
    importdata('train/5/xiajia.dat');
    importdata('train/6/xiajia.dat');
    importdata('train/7/xiajia.dat');
    importdata('train/8/xiajia.dat')];
% 
% A=[importdata('train/1/menban.dat');
%     importdata('train/2/menban.dat');
%     importdata('train/3/menban.dat');
%     importdata('train/4/menban.dat');
%     importdata('train/5/menban.dat');
%     importdata('train/6/menban.dat');
%     importdata('train/7/menban.dat');
%     importdata('train/8/menban.dat')];


n = size(A,1);
d = size(A,2)-2;
X = A(:,1:d);
y_bin = A(:,d+1);
y = A(:,d+2);
X = feature(X);
d = size(X,2);

R = 2.3;% max
y = y/ (2*R) + 0.5;
% 
n_train = floor(n*0.8);
X_train = X(1:n_train,1:d);
y_bin_train = y_bin(1:n_train);
y_train = y(1:n_train);
X_test = X(n_train+1:n,1:d);
y_bin_test = y_bin(n_train+1:n);
y_test = y(n_train+1:n);
n_test = size(X_test,1);
w  = train_logistic( X_train,y_train )
y_pred_train = predict(X_train,w);
y_pred_bin_train = y_pred_train>0.5;
L1_train = sum(abs(y_pred_train - y_train)) / n_train;
bin_loss_train = sum(abs(y_pred_bin_train - y_bin_train)) / n_train

y_pred_test = predict(X_test,w);
y_pred_bin_test = y_pred_test>0.5;
L1_test = sum(abs(y_pred_test - y_test)) /n_test
bin_loss_test = sum(abs(y_pred_bin_test - y_bin_test)) / n_test

%csvwrite('weight_dizhu.csv',w);
csvwrite('weight_xiajia.csv',w);
%csvwrite('weight_menban.csv',w);

% remain_train = min(X_train(:,15:17),[],2);
% 
% X_open = X_train(remain_train>10,:);
% y_open = y_train(remain_train>10,:);
% w_open  = train_logistic( X_open,y_open )
% 
% X_mid = X_train(((remain_train-11).* (remain_train-5) )<0,:);
% y_mid = y_train(((remain_train-11).* (remain_train-5) )<0,:);
% w_mid  = train_logistic( X_mid,y_mid )
% 
% X_end = X_train(remain_train<6 ,:);
% y_end= y_train(remain_train<6,:);
% w_end  = train_logistic( X_end,y_end )
% %

