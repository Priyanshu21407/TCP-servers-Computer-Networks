#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<stdbool.h>
#include<semaphore.h>

#define PORT 8080
#define CONNECTIONS 4000
#define REQUESTS 50

sem_t lock;

long int get_factorial (int n){
	long int ans=1;
	if(n>20) {
		n=20;
	}
	
	for(int i=1;i<=n;i++){
		ans=ans*i;
	}
	
	return ans;

}

void process_data(int client_sock){
	char recv_buffer[100];
	memset(recv_buffer, '\0', sizeof(recv_buffer));
	ssize_t recv_resp;
	
	printf("HERE\n");
	recv_resp = recv(client_sock, recv_buffer, sizeof(recv_buffer), 0);
	
	printf("client sent: %s\n", recv_buffer);
	printf("recv_resp %ld ISNULL %s, %ld:%d\n", recv_resp,recv_buffer, sizeof(recv_buffer), (recv_buffer=='\0'));
	if(recv_resp = recv(client_sock, recv_buffer, sizeof(recv_buffer), 0)>0){
	
	int n = atoi(recv_buffer);
	int ans = get_factorial(n);
	
	char response_to_client[1000];
	memset(response_to_client,'\0',sizeof(response_to_client));
	
	sprintf(response_to_client, "Factorial of %d is %d",n,ans);
	
	printf("response: %s\n",response_to_client);
	
	send(client_sock, response_to_client, sizeof(response_to_client),0);

	}
	else if(recv_resp==0){
		close(client_sock);
		exit(1);
	}
	else{
		close(client_sock);
		exit(1);
	}

	exit(0);
	
}


int main(){

	//signal(SIGPIPE, SIG_IGN);
	
	
	pid_t child;

	int server_sock, client_sock;
	struct sockaddr_in server_addr;
	server_sock = socket(AF_INET,SOCK_STREAM,0);
	if(server_sock==-1){
		printf("Error in creating socket\n");
		exit(1);
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);
	
	int bind_ = bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
	
	if(bind_<0){
		printf("Error in binding\n");
		exit(1);
	}
	
	int listen_ = listen(server_sock, CONNECTIONS);
	
	if(listen_<0){
		printf("Error in listening\n");
		exit(1);
	}
	printf("Listening for clients...\n");

	int count = CONNECTIONS;
	int num = 0;
	while(true){
		printf("COUNT: %d \n",count--);
		struct sockaddr_in client_addr;
		socklen_t addr_size = sizeof(client_addr);
		client_sock = accept(server_sock,(struct sockaddr*)&client_addr,&addr_size);
		
		if(client_sock<0){
			printf("Failed in connecting client\n");
			exit(1);
		}
	
		printf("New client connected: %d\n",num);
		
		if((child = fork())==0){	
			close(server_sock);	
			printf("Client process created\n");

		while(1){
		char recv_buff[100];
		memset(recv_buff, 0,sizeof(recv_buff));
		ssize_t recv_resp;
		
		
		
		printf("client sent: %s\n", recv_buff);

		if(recv_resp = recv(client_sock, recv_buff, sizeof(recv_buff), 0)>0){
		
		int n = atoi(recv_buff);
		long int ans = get_factorial(n);
		
		memset(recv_buff,0,sizeof(recv_buff));
		
		sprintf(recv_buff, "%ld",ans);
		
		printf("response: %s\n",recv_buff);
		
		send(client_sock, recv_buff, sizeof(recv_buff),0);

		}
		else if(recv_resp<=0){
			close(client_sock);
			exit(1);
			break;
		}

		}
			close(client_sock);
			exit(0);
			
		}
		else if(child<=0){
			perror("error creating child\n");
			close(client_sock);
			exit(1);
		}
		
		
		
	}
	
	close(server_sock);
	printf("server closed\n");

}

