function [ w ] = train_logistic( X,y, lambda )
% d = size(X,2);
% n = size(X,1);
% cvx_begin
%     variable w(d)
%     f = 1/n * ( (-y'*X*w+sum(log_sum_exp([zeros(1,n); (X*w)'])))) + lambda*(norm(w,2));
%     minimize f
% cvx_end
w = glmfit(X, y , 'binomial', 'link', 'logit');
% [w,FitInfo] = lassoglm(X, y , 'binomial', 'link', 'logit','Lambda',lambda);
% w0 = FitInfo.Intercept;
% w = [w0;w];
end

