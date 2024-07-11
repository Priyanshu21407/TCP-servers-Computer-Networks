#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <error.h>
#include <fcntl.h>
#include <sys/time.h>

#define PORT 8080
#define CONCURRENT 4000
#define REQUESTS 52
#define TIMEOUT 10

pthread_t threads[CONCURRENT];
int client_data_count[10000];
sem_t lock;
int total_send = 0;

uint64_t get_factorial(uint64_t n)
{
    uint64_t ans = 1;
    if (n > 20)
    {
        n = 20;
    }

    for (int i = 1; i <= n; i++)
    {
        ans = ans * i;
    }

    return ans;
}

void *client_process(void *args)
{

    int client_sock = *(int *)args;
    printf("client accepted: %d\n", client_sock);

    int curr_req=0;
    char recv_buffer[1024];
    for (curr_req ; curr_req<REQUESTS; curr_req++)
    {
        memset(recv_buffer, '\0', sizeof(recv_buffer));

        int recv_no;
        if (recv_no = recv(client_sock, recv_buffer, sizeof(recv_buffer), 0) > 0)
        {
            recv_buffer[recv_no] = '\0';

            printf("client sent: %s\n", recv_buffer);

            uint64_t n = atoll(recv_buffer);
            uint64_t ans = get_factorial(n);

            memset(recv_buffer, '\0', sizeof(recv_buffer));

            sprintf(recv_buffer, "%lu", ans);

            printf("response: %s\n", recv_buffer);

            if(send(client_sock, recv_buffer, sizeof(recv_buffer), 0)<0){
                perror("error sending");
                continue;

            }

            
        }

        else if (recv_no <= 0)
        {
            perror("client closed connections\n");
            close(client_sock);
            break;
        }
    }

}

int main()
{

    // signal(SIGPIPE, SIG_IGN);

    sem_init(&lock, 0, 1);
    int server_sock;
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        printf("Error creating server socket\n");
        exit(1);
    }
    printf("server socket created\n");

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);
    socklen_t server_addr_size = sizeof(server_addr);

    int bind_;
    if ((bind_ = bind(server_sock, (struct sockaddr *)&server_addr, server_addr_size)) < 0)
    {
        perror("Error in binding\n");
        exit(1);
    }

    int listen_;

    if ((listen_ = listen(server_sock, CONCURRENT)) < 0)
    {
        printf("Error in listening\n");
        exit(1);
    }
    printf("Listening...\n");

    int client_no = 0;
    int count = CONCURRENT + 1;
    int num = 0;
    while (1)
    {
        int client_sock;
        struct sockaddr_in client_addr;
        socklen_t addr_size = sizeof(client_addr);

        if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_size)) < 0)
        {
            perror("Error in connecting to client\n");
            exit(1);
        }
        printf("New client connected\n");
        
        int *arg = (int *)malloc(sizeof(int));
        *arg = client_sock;

        if (pthread_create(&threads[client_no++], NULL, client_process, (void *)arg) != 0)
        {
            perror("create error");
        }
        pthread_detach(threads[client_no - 1]);
    }

 
    close(server_sock);

    printf("server closed\n");
}
