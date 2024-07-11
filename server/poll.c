#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<poll.h>
#include<unistd.h>
#include<stdbool.h>


#define PORT 8080
#define CONNECTIONS 4000


int get_factorial (int n){
	int ans=1;
	if(n>20) {
		n=20;
	}
	
	for(int i=1;i<=n;i++){
		ans=ans*i;
	}
	
	return ans;

}

bool process_data(int client_sock){
	char recv_buffer[1000];
	memset(recv_buffer, '\0', sizeof(recv_buffer));
	ssize_t recv_resp;
	recv_resp = recv(client_sock, recv_buffer, sizeof(recv_buffer), 0);
	
	printf("client sent: %s\n", recv_buffer);
	printf("recv_resp %ld ISNULL %s, %ld:%d\n", recv_resp,recv_buffer, sizeof(recv_buffer), (recv_buffer=='\0'));
	if(recv_resp<=0){
		printf("client closed the connection\n");
		return false;
	}

	int n = atoi(recv_buffer);
	int ans = get_factorial(n);
	
	char response_to_client[1000];
	memset(response_to_client,'\0',sizeof(response_to_client));
	
	sprintf(response_to_client, "Factorial of %d is %d",n,ans);
	
	printf("response: %s\n",response_to_client);
	
	send(client_sock, response_to_client, sizeof(response_to_client),0);
	
	return true;
	
	
}


int main(){
	signal(SIGPIPE, SIG_IGN);
	
	int server_sock;
	struct sockaddr_in server_addr;
	
	if((server_sock = socket(AF_INET, SOCK_STREAM,0))<0){
		perror("error creating server socket\n");
		exit(1);
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	int bind_;
	if((bind_ = bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)))<0){
		perror("error in binding\n");
		exit(1);
	}
	
	int listen_;
	if((listen_ = listen(server_sock,CONNECTIONS))<0){
		perror("error in listening\n");
	}
	printf("listening...\n");
	
	struct pollfd *fd_polls;
	fd_polls = malloc(CONNECTIONS * sizeof(struct pollfd));
	int fd_num = 1;
	nfds_t fdn;
	fd_polls->fd = server_sock;
	fd_polls->events = POLLIN;
	fd_polls->revents = 0;
	
	
	struct sockaddr_in client_addr;
	socklen_t addr_size = sizeof(client_addr);
	
	while(true){
		fdn = fd_num;

		int poll_;
		if((poll_ = poll(fd_polls,fdn,-1))<0){
			perror("error polling\n");
			exit(1);
		}
		printf("poll response received\n");
		for(int i=0;i<=fdn;i++){
		if((fd_polls + i)->revents & POLLIN == POLLIN){
			if((fd_polls + i)->fd >0){
				if((fd_polls + i)->fd == server_sock){
					int client_;
					if((client_ = accept(server_sock, (struct sockaddr*) &client_addr, &addr_size)) < 0){
						perror("error accepting client\n");
						exit(1);
					}
					printf("client accepted %d\n",client_);
					(fd_polls + fd_num)->fd = client_;
					(fd_polls + fd_num)->events = POLLIN;
					(fd_polls + fd_num)->revents = 0;
					fd_num++;

				}
				else{
				
					printf("processing data\n");
					if(!process_data((fd_polls + i)->fd)){
						(fd_polls + i)->fd = -1;
						close((fd_polls + i)->fd);
						
					}
					
				}
				
			}
		}
		}
		
	
	
	}
	printf("server closing...\n");
	
	sleep(5);
	
	close(server_sock);
	printf("server closed\n");
	
	return 0;
	
}
