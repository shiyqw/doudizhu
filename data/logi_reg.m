%A=importdata('greedy/v1/dizhu.dat');
%A=importdata('greedy/v1/xiajia.dat');
A=importdata('greedy/v1/menban.dat');
n = size(A,1);
d = size(A,2)-1;
X = A(:,1:d);
y = A(:,d+1);
X = feature(X);
d = size(X,2);


remain = min(X(:,15:17),[],2);

n_train = floor(n*0.8);
X_train = X(1:n_train,1:d);
y_train = y(1:n_train);
X_test = X(n_train+1:n,1:d);
y_test = y(n_train+1:n);
n_test = size(X_test,1);
w  = train_logistic( X_train,y_train )
y = predict(X_train,w);
y = (y>0.5);
acc_train = 1 - sum(abs(y - y_train)) / n_train

y = predict(X_test,w);
y = (y>0.5);
acc_test = 1 - sum(abs(y - y_test)) /n_test 

%csvwrite('weight_dizhu.csv',w);
%csvwrite('weight_xiajia.csv',w);
csvwrite('weight_menban.csv',w);

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

