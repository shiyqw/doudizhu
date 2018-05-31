function [J, grad] = costFunctionReg(W, X, y, rem,lambda)
%COSTFUNCTIONREG Compute cost and gradient for logistic regression with regularization
%   J = COSTFUNCTIONREG(theta, X, y, lambda) computes the cost of using
%   theta as the parameter for regularized logistic regression and the
%   gradient of the cost w.r.t. to the parameters. 

    % Initialize some useful values
    m = length(y); % number of training examples

    % You need to return the following variables correctly 
    J = 0; 
    grad = zeros(size(W));
    
      % hypothesis logistic regression
    reg_term = 0;
    for k = 1:19
        X_k =  X(rem(:,1)==k,1:22);
        nk = size(X_k,1);
        X_k = [ones(nk,1) X_k];
        y_k = y(rem(:,1)==k);
        sig = 1./(1+exp(-X_k * W(:,k))); 
        J = J + sum((-y_k .* log(sig)) - ((1 - y_k) .* log(1 - sig)))/m ;
        grad(:,k) = grad(:,k) + (X_k' * (sig - y_k) ./ m);
        if k>1
            reg_term  = reg_term + norm(W(:,k)-W(:,k-1),2)^2*lambda / (2 * m);
            grad(:,k) = grad(:,k) + (W(:,k)-W(:,k-1))*lambda/m;
        end
        if k<19
            grad(:,k) = grad(:,k) + (W(:,k)-W(:,k+1))*lambda/m;
        end
    end
    J = J+reg_term;
end