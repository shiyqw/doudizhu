%A=importdata('greedy/v2/dizhu.dat');
%A=importdata('greedy/v2/xiajia.dat');
%pre_processing;
X_train = csvread('x_train_dizhu.csv');
y_train = csvread('y_train_dizhu.csv');
X_test = csvread('x_test_dizhu.csv');
y_test = csvread('y_test_dizhu.csv');
[x,t] = simplefit_dataset;
net = feedforwardnet([5]);
net = train(net,X_train',y_train');
view(net)
y_pred_train = net(X_train')';
y_pred_test = net(X_test')';
perf = perform(net,y_test',y_pred_test')
figure(1)
scatter(y_train,y_pred_train,'.')
figure(2)
scatter(y_test,y_pred_test,'.')
% y_pred_train = predict(X_train,w);
% y_pred_bin_train = y_pred_train>0.5;
% L1_train = sum(abs(y_pred_train - y_train)) / n_train
% %bin_loss_train = sum(abs(y_pred_bin_train - y_bin_train)) / n_train
% figure(1)
% scatter(y_pred_train,y_train,'.')
% y_pred_test = predict(X_test,w);
% y_pred_bin_test = y_pred_test>0.5;
% figure(2)
% scatter(y_pred_test,y_test,'.')
% L1_test = sum(abs(y_pred_test - y_test)) /n_test
%bin_loss_test = sum(abs(y_pred_bin_test - y_bin_test)) / n_test

%csvwrite('weight_dizhu.csv',w);
%csvwrite('weight_xiajia.csv',w);

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

