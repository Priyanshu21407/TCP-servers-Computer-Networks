#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/select.h>
#include<unistd.h>
#include<stdbool.h>
#include<sys/time.h>


#define PORT 8081
#define CONNECTIONS 1000
#define REQUESTS 52
#define TIMEOUT 5


int client_isset[CONNECTIONS];


void add_client(int new){
	for(int i=0;i<CONNECTIONS;i++){
		if(client_isset[i]==-1){
			client_isset[i] = new;
			break;
		}
	}

}

int setmax_fd(int x){
	for(int i=0;i<CONNECTIONS;i++){
		if(x<client_isset[i]){
			x=client_isset[i];
		}
	}
	return x;
}

uint64_t get_factorial (uint64_t n){
	uint64_t ans=1;
	if(n>20) {
		n=20;
	}
	
	for(int i=1;i<=n;i++){
		ans=ans*i;
	}
	
	return ans;

}

bool process_data(int client_sock){
	char recv_buffer[10000];
	memset(recv_buffer, '\0', sizeof(recv_buffer));
	ssize_t recv_resp;
	recv_resp = recv(client_sock, recv_buffer, sizeof(recv_buffer), 0);
	
//	printf("client sent: %d\n", recv_buffer==NULL);

	if(recv_resp<=0){
//		printf("client closed the connection\n");
		return false;
	}

	int n = atoi(recv_buffer);
	long int ans = get_factorial(n);
	
	char response_to_client[10000];
	memset(response_to_client,'\0',sizeof(response_to_client));
	
	sprintf(response_to_client, "Factorial of %d is %ld",n,ans);
	
//	printf("response: %s\n",response_to_client);
	
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
//		perror("error in binding\n");
		exit(1);
	}
	
	int listen_;
	if((listen_ = listen(server_sock,CONNECTIONS))<0){
//		perror("error in listening\n");
	}
	//printf("listening...\n");
	
	fd_set clients, updated_clients;
	int max_fd=server_sock;
	FD_ZERO(&clients);
	FD_SET(server_sock,&clients);
	struct sockaddr_in client_addr;
	socklen_t addr_size = sizeof(client_addr);
	

	
	int num=0,sendno=0;
	struct timeval tout;
	tout.tv_usec = 0;
	tout.tv_sec = 5;
	memset(client_isset,-1,sizeof(client_isset));
	
	while(true){
		updated_clients = clients;
		
		max_fd = setmax_fd(max_fd);
		
		int select_;
		if(select_ = select(max_fd+1,&updated_clients, NULL, NULL, NULL)<0){
//			printf("select return : %d\n",select_);
//			perror("error selecting\n");
			exit(1);
		}
//		printf("select response received %d\n",select_);
		int iter = 0;
		
		if(FD_ISSET(server_sock,&updated_clients)){
			int client_;
			if((client_ = accept(server_sock, (struct sockaddr*) &client_addr, &addr_size)) < 0){
				
				//perror("error accepting client\n");
				exit(1);
			}
			//printf("client accepted %d\n",client_);
			FD_SET(client_,&clients);
			if(max_fd<client_) max_fd = client_;
			add_client(client_);
		}
		
		for(int iter_no=0;iter_no<CONNECTIONS;iter_no++){
		int i = client_isset[iter_no];
			if(FD_ISSET(i,&updated_clients) && i>0){	
		//printf("client sock : %d\n",i);
					int client_sock = i;
					for(int curr_req=0; curr_req<REQUESTS;curr_req++){
					char recv_buffer[1024];
					memset(recv_buffer,0, sizeof(recv_buffer));
					int recv_resp;
					recv_resp = recv(client_sock, recv_buffer, sizeof(recv_buffer), 0);
					
					//printf("recv_resp %d\n",recv_resp);
					
					//printf("client sent: %s\n", recv_buffer);

					if(recv_resp==0){
					//printf("CLEARING....\n");
						FD_CLR(i,&clients);
						client_isset[iter_no] = -1;
						break;
					}
					else if(recv_resp<0){
						//perror("error recv\n");
						client_isset[iter_no] = -1;
						break;
					}

					uint64_t n = atoll(recv_buffer);
					uint64_t ans = get_factorial(n);
					
					memset(recv_buffer,0,sizeof(recv_buffer));
					
					sprintf(recv_buffer, "%ld",ans);
					
					//printf("response: %s\n",recv_buffer);
					
					send(client_sock, recv_buffer, sizeof(recv_buffer),0);
					//printf("SEND NO : %d\n",sendno++);
					
				}
				
			}
		}
		
	
	
	}
	
	close(server_sock);
	
	return 0;
	
}
