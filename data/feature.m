function [ feat ] = feature(X)
n = size(X,1);
d = size(X,2);
%X = X(:,1:20);

% remain = X(:,16:18);
% inv_remain = (remain+0.01).^(-1);
% 
% 
% pairs = sum((X(:,1:13)==2),2);
% triplets = sum((X(:,1:13)==3),2);
%feat = [X pairs triplets inv_remain ];
%feat = [X inv_remain ];
feat = [X];
d = size(feat,2);
feat = feat(:,2:d);
end

