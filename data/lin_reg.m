X_train = csvread('x_train_dizhu.csv');
y_train = csvread('y_train_dizhu.csv');
X_test = csvread('x_test_dizhu.csv');
y_test = csvread('y_test_dizhu.csv');
w = glmfit(X, y , 'normal', 'link', 'identity')
y_pred_train = predict_lin(X_train,w);
y_pred_bin_train = y_pred_train>0.5;
L2_train = sum((y_pred_train - y_train).^2) / n_train
%bin_loss_train = sum(abs(y_pred_bin_train - y_bin_train)) / n_train
figure(1)
scatter(y_train,y_pred_train,'.')
y_pred_test = predict_lin(X_test,w);
y_pred_bin_test = y_pred_test>0.5;
figure(2)
scatter(y_test,y_pred_test,'.')
L2_test = sum((y_pred_test - y_test).^2) /n_test