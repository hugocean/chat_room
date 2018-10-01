# chat_room
即时通讯系统

这个项目分为两大部分：服务器端和客户端
一、客户端负责给用户提供接口，发送请求给服务器端，并接收来自服务器端的消息。
所以服务器端需要两个线程一个专门负责发送，一个专门负责接收！

二、服务器端则需要随时接收客户端的连接请求，在没接收到一个客户端的连接请求时就要单独创建一个线程来

服务器端：
1.创建socket
sockfd = socket(PF_INET, SOCK_STREAM, 0); 
这里第二个参数是创建socket流，用于TCP服务器，UDP的话是
2.绑定
这里用到了一个结构体
memset(&server_addr, 0, sizeof(server_addr));
server_addr.sin_family = PF_INET;
server_addr.sin_port = htons(PORT);
server_addr.sin_addr.s_addr = inet_addr("192.168.238.129");
bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
3.监听
listen(sockfd, 5);
4.接收
fd[i] = accept(sockFd, (struct sockaddr *)&client_addr, &length);
5.创建线程
pthread_create(&tid, NULL, ClientHandler, &fd[i]);
6.发送/接收
这里的RecvInfo，TempInfo都是结构体，服务器和客户端都要定义，平且要一样
recv(fd ,&RecvInfo, sizeof(RecvInfo), 0);
send(tofd, &TempInfo, sizeof(TempInfo), 0);

客户端：
1.创建socket
sockfd = socket(PF_INET, SOCK_STREAM, 0);
2.连接
memset(&server_addr, 0, sizeof(server_addr));
server_addr.sin_family = PF_INET;
server_addr.sin_port = htons(PORT);
server_addr.sin_addr.s_addr = inet_addr("192.168.238.129");
connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
3.接收发送

