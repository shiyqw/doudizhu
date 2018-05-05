function [ w ] = train_logistic( X,y )
w = glmfit(X, y , 'binomial', 'link', 'logit');
end

