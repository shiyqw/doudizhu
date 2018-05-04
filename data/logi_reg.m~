A=importdata('xiajia.dat');
X = A(:,2:18);
y = A(:,19);
n = size(A,1);
d = 17;
n_train = floor(0.8 * n);
X_train = A(1:n_train,2:18);
y_train = A(1:n_train,19);
X_test = A(n_train+1:n,2:18);
y_test = A(n_train+1:n,19);
n_test = size(X_test,1);

X_train = X_train(1:n_train,:);
y_train = y_train(1:n_train,:);


w = glmfit(X, y , 'binomial', 'link', 'logit')