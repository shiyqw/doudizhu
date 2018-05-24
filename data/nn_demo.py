import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
import pandas as pd
import numpy as np
from torch.autograd import Variable
from torchvision import datasets, transforms
import torch.utils.data as data_utils

def create_nn(batch_size=1, learning_rate=0.003, epochs=10,
              log_interval=1000):
    x_train = torch.from_numpy(np.array(pd.read_csv('x_train_dizhu.csv')))
    y_train = torch.from_numpy(np.array(pd.read_csv('y_train_dizhu.csv')))
    x_train, y_train = x_train.type(torch.FloatTensor),y_train.type(torch.FloatTensor)
    x_test = torch.from_numpy(np.array(pd.read_csv('x_test_dizhu.csv')))
    y_test = torch.from_numpy(np.array(pd.read_csv('y_test_dizhu.csv')))
    x_test, y_test = x_test.type(torch.FloatTensor),y_test.type(torch.FloatTensor)
    train = data_utils.TensorDataset(x_train, y_train)
    train_loader = data_utils.DataLoader(train, batch_size, shuffle=True)
    test = data_utils.TensorDataset(x_test, y_test)
    test_loader = data_utils.DataLoader(test, batch_size, shuffle=True)
    print(len(train_loader.dataset))
    print(len(test_loader.dataset))
    class Net(nn.Module):
        def __init__(self):
            super(Net, self).__init__()
            self.fc1 = nn.Linear(52, 5)
            self.fc2 = nn.Linear(5,5)
            self.fc3 = nn.Linear(5,1)

        def forward(self, x):
            x = F.relu(self.fc1(x))
            x = F.relu(self.fc2(x))
            x = self.fc3(x)
            return x
    net = Net()
    print(net)
    # create a stochastic gradient descent optimizer
    optimizer = optim.SGD(net.parameters(), lr=learning_rate)
    # create a loss function
    criterion = nn.MSELoss()
    test_loss = 0
    for data, target in test_loader:
        data, target = Variable(data), Variable(target)
        net_out = net(data)
        # sum up batch loss
        test_loss += criterion(net_out, target).item()
        #pred = net_out.data.max(1)[1]  # get the index of the max log-probability
        #correct += pred.eq(target.data).sum()
    test_loss /= len(test_loader.dataset)
    print('\nTest set: Average loss: {:.4f}\n'.format(test_loss))
    # run the main training loop
    for epoch in range(epochs):
        for batch_idx, (data, target) in enumerate(train_loader):
            data, target = Variable(data), Variable(target)
            #data = data.view(-1, 28*28)
            optimizer.zero_grad()
            net_out = net(data)
            loss = criterion(net_out, target)
            loss.backward()
            optimizer.step()
            if batch_idx % log_interval == 0:
                print('Train Epoch: {} [{}/{} ({:.0f}%)]\tLoss: {:.6f}'.format(
                    epoch, batch_idx * len(data), len(train_loader.dataset),
                           100. * batch_idx / len(train_loader), loss.item()))
        '''
        train_loss = 0
        for data, target in train_loader:
            data, target = Variable(data), Variable(target)
            net_out = net(data)
            # sum up batch loss
            train_loss += criterion(net_out, target).item()
            #pred = net_out.data.max(1)[1]  # get the index of the max log-probability
            #correct += pred.eq(target.data).sum()
        train_loss /= len(train_loader.dataset)
        print('\nTrain set: Average loss: {:.4f}\n'.format(train_loss))
        '''
        test_loss = 0
        for data, target in test_loader:
            data, target = Variable(data), Variable(target)
            net_out = net(data)
            # sum up batch loss
            test_loss += criterion(net_out, target).item()
            #pred = net_out.data.max(1)[1]  # get the index of the max log-probability
            #correct += pred.eq(target.data).sum()
        test_loss /= len(test_loader.dataset)
        print('\nTest set: Average loss: {:.4f}\n'.format(test_loss))


if __name__ == "__main__":
    create_nn()

