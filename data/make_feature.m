function [ feat ] = make_feature(X)
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
%feat = [X];
feat = [X(:,1:15), X(:, 46:53), sum(X(:,1:15),2),sum(X(:,16:30),2),sum(X(:,31:45),2)];
% feat = [feat, feat.^2];
% for i = 1:15
%     feat = [feat, feat(:,i).*feat(:, i+15)];
% end
%feat = [X(:,1:15),  X(:, 46:53)];
d = size(feat,2);
feat = feat(:,2:d);
end

