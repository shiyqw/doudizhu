function [ y ] = predict_piece_wise(X, W)
n = size(X, 1);
y = zeros(n,1);
for i=1:n
    x = [1 X(i,1:22)];
    r = round(X(i,23));
    if(r==0)
        y(i) = 1e9;
    else
        y(i) = x*W(:, r);
    end
end
y = 1./(exp(-y)+1);
end

