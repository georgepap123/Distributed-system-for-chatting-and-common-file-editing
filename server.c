#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

char *name;
int port = 5555;
int clients = 5;
int active_clients = 0;
#define FILENAME "servers_file.txt"

typedef struct
{
    char name[20];
    int port;
    char message[4096];
    int timestamp;
    int file;
    int port_from;

} Message;

typedef struct
{
    int go;
    int correct_timestamp;
}Check;

typedef struct
{
    int* client_port;
    int* client_timestamp;
}Client_info;

void receive_message(int server_fd);
void *receive_thread(void *server_fd);

Client_info client_info;
Check check;
int main(int argc,char const *argv[])
{

    
    client_info.client_port=(int*)malloc(sizeof(int)*clients);
    client_info.client_timestamp=(int*)malloc(sizeof(int)*clients);
    
    memset(client_info.client_port, 0, sizeof(int) * clients);
    memset(client_info.client_timestamp, 0, sizeof(int) * clients);
   
   //print gia arxikopoiisi tou dinamikou pinaka testing
   /* for(int i = 0; i<clients; i++)
    {
        printf("client_port[%d] = %d\n",i, client_info.client_port[i]);
        printf("client_timestamp[%d] = %d\n",i, client_info.client_timestamp[i]);
    }
    */

    int server_fd, new_socket, valread;
    struct sockaddr_in address;

    //Dimiourgia socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM,0)) == 1)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port=htons(port);

    
    printf("Server's IP address: %s \n",inet_ntoa(address.sin_addr));
    printf("Server's PORT is:%d\n",(int)ntohs(address.sin_port));

    if(bind(server_fd,(struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if(listen(server_fd, 5) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

  
    int choice = 1;
    pthread_t tid;
    pthread_create(&tid, NULL, &receive_thread, &server_fd);
    printf("Server is active\n"); 

    while(choice !=0)
    {
        printf("press 0 to close the server \n");
        scanf("%d", &choice);
        if (choice != 0)
        {
            printf("\nwrong choice\n");
        }
    }

    if(choice == 0)
    {
        printf("\n Closing \n");
        close(server_fd);
    }

    return 0;
}

void *receive_thread(void *server_fd)
{
    int s_fd = *((int *)server_fd);

    while(1)
    {
        sleep(2);
        receive_message(s_fd);
    }
}

void receive_message(int server_fd)
{
    struct sockaddr_in address;
    int valread;
    int addrlen = sizeof(address);
    fd_set current_sockets, ready_sockets;

    FD_ZERO(&current_sockets);
    FD_SET(server_fd, &current_sockets);
    int k = 0;
    int exists = 0;
    while (1)
    {
    
    struct sockaddr_in address;
    int valread;
    int addrlen = sizeof(address);
    fd_set current_sockets, ready_sockets;

    FD_ZERO(&current_sockets);
    FD_SET(server_fd, &current_sockets);
    int k = 0;

    while (1)
    {
        exists = 0;
        k++;
        ready_sockets = current_sockets;

        if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0)
        {
            perror("Error");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < FD_SETSIZE; i++)
        {
            if (FD_ISSET(i, &ready_sockets))
            {
                if (i == server_fd)
                {
                    int client_socket;

                    if ((client_socket = accept(server_fd, (struct sockaddr *)&address,
                                                (socklen_t *)&addrlen)) < 0)
                    {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }
                    FD_SET(client_socket, &current_sockets);
                }
                else
                {
                    Message message;
                    memset(&message, 0, sizeof(Message));
                    valread = recv(i, &message, sizeof(Message), 0);
                    if (valread == 0)
                    {
                        // Client closed the connection, skip processing the message
                        FD_CLR(i, &current_sockets);
                        close(i);
                        continue;
                    }
                    for (int j = 0; j < active_clients; j++)
                    {   
                 
                    if (message.port_from == client_info.client_port[j])
                    {
                            
                    exists = 1;
                    if (message.timestamp == client_info.client_timestamp[j])
                    {
                        
                        // test minima gia periptosi 1
                        //printf("bika matching client_port matching timestamp\n");
                        
                        check.go =1;
                        check.correct_timestamp = client_info.client_timestamp[j];
                        client_info.client_timestamp[j]++;
                      
                        break;
                    }
                    else if(message.timestamp != client_info.client_timestamp[j])
                    {
                        // test minima gia periptosi 2
                        //printf("bika matching client_port not matching timestamp\n");
                       
                        check.go =1;
                        check.correct_timestamp = client_info.client_timestamp[j];
                        client_info.client_timestamp[j]++;
                     
                        break;
                    }
                    }
                    }

                       if(exists == 0)
                        {
                            
                            if(active_clients == clients)
                            {
                                //testing gia realloc
                                //printf("realloc clients list \n");
                                
                                int* temp = realloc(client_info.client_port,sizeof(int)*(clients*2));
                                client_info.client_port = temp;
                                free(temp);
                                temp = realloc(client_info.client_timestamp,sizeof(int)*(clients*2));
                                client_info.client_timestamp = temp;
                                free(temp);
                                clients = clients*2;
                            }
                            client_info.client_port[active_clients] = message.port_from;
                            
                            
                            if(message.timestamp == client_info.client_timestamp[active_clients])
                            {
                                // test minima gia periptosi 3
                                //printf("not matching client  matching timestamp\n");
                                
                                check.go = 1;
                                check.correct_timestamp =client_info.client_timestamp[active_clients];
                                client_info.client_timestamp[active_clients]++;
                             
                            }
                            else
                            {  
                                // test minima gia periptosi 4
                                //printf("not matching client not matching timestamp\n");
                                
                                check.go =1;
                                check.correct_timestamp = client_info.client_timestamp[active_clients];
                                client_info.client_timestamp[active_clients]++;
                                   
                            }
                            active_clients++;
                        }
 
                         check.go =1;
                         
                         //want to send back the check to the client he received the message
                             if(send(i, (void*)&check, sizeof(check),0) == -1)
                             {
                                perror("error sending back the message");

                                break;
                            }

                        FILE *file =fopen(FILENAME, "a");
                        if (file == NULL)
                        {
                            printf("Error opening the file\n");
                            break;
                        }
                       if(message.file == 1)
                    {
                       
                        printf("user %s from port %d, timestamp %d, updated the file \n",message.name, message.port_from,check.correct_timestamp);
                       
                        fprintf(file, "user %s from port %d, timestamp %d, updated the file \n",message.name,message.port_from,check.correct_timestamp);
                         
                       
                    }
                    else if(message.file == 0)
                    {
                        printf("\nUser:%s from port %d and timestamp:%d sent message: %s to port %d\n",message.name, message.port_from, check.correct_timestamp, message.message, message.port);
                       
                        fprintf(file, "User:%s from port %d and timestamp:%d sent message: %s to port %d\n",message.name, message.port_from, check.correct_timestamp, message.message, message.port);
                        
                    
                    }
                    fclose(file); 
                    FD_CLR(i, &current_sockets);
                }
               if (k == (FD_SETSIZE * 2))
                 break;
                 
            }
              
        }

      
    }
    }
}