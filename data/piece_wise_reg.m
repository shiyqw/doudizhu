pre_processing;
X_train = csvread('x_train_dizhu.csv');
y_train = csvread('y_train_dizhu.csv');
X_test = csvread('x_test_dizhu.csv');
y_test = csvread('y_test_dizhu.csv');
y_bin_train = csvread('y_bin_train_dizhu.csv');
y_bin_test = csvread('y_bin_test_dizhu.csv');
n_train = size(X_train,1);
n_test = size(X_test,1);
rem = X_train(:,23:25);
lambda = 1e4;
W= zeros(23,19);
%W = csvread('piecewise_weight_dizhu.csv');
lr = 1;
% for t = 1:1000   
%     [J, grad] = costFunctionReg(W, X_train,y_train, rem, lambda);
%     W = W - lr*grad;
%     if sum(sum(grad.^2)) < 1e-6
%         t
%         break;
%     end
%     W = sign(W).*(max((abs(W) - 0.001),0));
%     J
% end

w = train_logistic( X_train(:,1:22),y_train, 0.005 );
W = w*ones(1,19);
% for k = 1:19
% X_train_k =  X_train(rem(:,1)==k,1:22);
% y_train_k = y_train(rem(:,1)==k,:);
% nk = size(X_train_k,1);
% lambda = 0;
% w = train_logistic( X_train_k ,y_train_k );
% W(:,k)=w;
% end
%csvwrite('piecewise_weight_dizhu.csv',W);
y_pred_train = predict_piece_wise(X_train,W);
y_pred_bin_train = y_pred_train>0.5;
y_pred_test = predict_piece_wise(X_test,W);
y_pred_bin_test = y_pred_test>0.5;
train_loss = cross_ent(y_train, y_pred_train)
wl_acc_train = 1-sum(abs(y_pred_bin_train - y_bin_train)) / n_train
%corre_analysis(y_pred_train ,y_bin_train)
test_loss = cross_ent(y_test, y_pred_test)
wl_acc_test = 1- sum(abs(y_pred_bin_test - y_bin_test)) / n_test

% figure(1)
% n = hist3([y_train, y_pred_train]);
% pcolor(n)
% y_pred_test = predict_piece_wise(X_test,W);
% y_pred_bin_test = y_pred_test>0.5;
% figure(2)
% n = hist3([y_test, y_pred_test]);
% pcolor(n)

% X_train = csvread('x_train_xiajia.csv');
% y_train = csvread('y_train_xiajia.csv');
% X_test = csvread('x_test_xiajia.csv');
% y_test = csvread('y_test_xiajia.csv');
% y_bin_train = csvread('y_bin_train_xiajia.csv');
% y_bin_test = csvread('y_bin_test_xiajia.csv');
% n_train = size(X_train,1);
% n_test = size(X_test,1);
% rem = X_train(:,23:25);
% W= zeros(23,19);
% %W = csvread('piecewise_weight_dizhu.csv');
% lr = 0.3;
% for t = 1:10000    
%     [J, grad] = costFunctionReg(W, X_train,y_train, rem, lambda);
%     W = W - lr*grad;
%     if sum(sum(grad.^2)) < 1e-6
%         t
%         break;
%     end
%     J
% end
% % W = [];
% % for k = 1:19
% % X_train_k =  X_train(rem(:,1)==k,1:22);
% % y_train_k = y_train(rem(:,1)==k,:);
% % w = train_logistic( X_train_k ,y_train_k );
% % W = [W w];
% % end
% csvwrite('piecewise_weight_xiajia.csv',W);
% y_pred_train = predict_piece_wise(X_train,W);
% y_pred_bin_train = y_pred_train>0.5;
% train_loss = cross_ent(y_train, y_pred_train)
% wl_acc_train = 1-sum(abs(y_pred_bin_train - y_bin_train)) / n_train
% y_pred_test = predict_piece_wise(X_test,W);
% y_pred_bin_test = y_pred_test>0.5;
% %corre_analysis(y_pred_train ,y_bin_train)
% test_loss = cross_ent(y_test, y_pred_test)
% wl_acc_test = 1- sum(abs(y_pred_bin_test - y_bin_test)) / n_test




% X_train = csvread('x_train_menban.csv');
% y_train = csvread('y_train_menban.csv');
% X_test = csvread('x_test_menban.csv');
% y_test = csvread('y_test_menban.csv');
% y_bin_train = csvread('y_bin_train_menban.csv');
% y_bin_test = csvread('y_bin_test_menban.csv');
% n_train = size(X_train,1);
% n_test = size(X_test,1);
% rem = X_train(:,23:25);
% W= zeros(23,19);
% %W = csvread('piecewise_weight_dizhu.csv');
% lr = 0.3;
% for t = 1:10000    
%     [J, grad] = costFunctionReg(W, X_train,y_train, rem, lambda);
%     W = W - lr*grad;
%     if sum(sum(grad.^2)) < 1e-6
%         t
%         break;
%     end
%     J
% end
% % W = [];
% % for k = 1:19
% % X_train_k =  X_train(rem(:,1)==k,1:22);
% % y_train_k = y_train(rem(:,1)==k,:);
% % w = train_logistic( X_train_k ,y_train_k );
% % W = [W w];
% % end
% csvwrite('piecewise_weight_menban.csv',W);
% y_pred_train = predict_piece_wise(X_train,W);
% y_pred_bin_train = y_pred_train>0.5;
% train_loss = cross_ent(y_train, y_pred_train)
% wl_acc_train = 1-sum(abs(y_pred_bin_train - y_bin_train)) / n_train
% %corre_analysis(y_pred_train ,y_bin_train)
% y_pred_test = predict_piece_wise(X_test,W);
% y_pred_bin_test = y_pred_test>0.5;
% test_loss = cross_ent(y_test, y_pred_test)
% wl_acc_test = 1- sum(abs(y_pred_bin_test - y_bin_test)) / n_test
