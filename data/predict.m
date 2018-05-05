function [ y ] = predict(X, w )
n = size(X, 1);
y = [ones(n,1) X]*w;
y = 1./(exp(-y)+1);
end

