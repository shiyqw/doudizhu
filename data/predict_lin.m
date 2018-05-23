function [ y ] = predict_lin(X, w )
n = size(X, 1);
y = [ones(n,1) X]*w;
end


