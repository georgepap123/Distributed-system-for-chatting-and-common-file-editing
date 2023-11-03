#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

char* name; //onoma xristi (id)
int PORT; //port sindesis
int logical_clock = 0; // to thelo gia: algorithmos lamport gia total order multicast
int servers_port = 5555; //server's port
#define MAX_FILE_SIZE 4096
#define FILENAME "shared_file.txt"

//struct minimatos
typedef struct
{
    char name[20];
    int port;
    char message[MAX_FILE_SIZE];
    int timestamp;
    int file;
    int port_from;
} Message;


typedef struct
{
    char name[20];
    int port;
    char content[MAX_FILE_SIZE];
    int timestamp;
    int type;
} SharedFile;

typedef struct
{
    int go;
    int correct_timestamp;

}Check;

void send_message();
void receive_message(int server_fd);
void *receive_thread(void *server_fd);
void total_order_multicast(Message *message);
void load_local_file(const char* filename, SharedFile* shareFile);
void print_local_file(SharedFile* shareFile);
void edit_local_file(SharedFile *sharefile);
void send_local_file(SharedFile* sharefile);


int main(int argc, char const *argv[])
{
    name = (char*)malloc(20 * sizeof(char));
    printf("Enter name:");
    scanf("%s", name);

    printf("Enter your port number:");
    scanf("%d", &PORT);

    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int k = 0;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    printf("IP address is: %s\n", inet_ntoa(address.sin_addr));
    printf("Port is: %d\n", (int)ntohs(address.sin_port));

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    SharedFile sharedFile;
    memset(sharedFile.content, 0 , sizeof(sharedFile.content));
    sharedFile.timestamp = 0;
    load_local_file(FILENAME, &sharedFile);
    int choice = 3;
    pthread_t tid;
    pthread_create(&tid, NULL, &receive_thread, &server_fd);
    printf("\n-------Kalos irthes sto katanemimeno sistima------- \n");

    while (choice != 0)
    {
        printf("1. Send message");
        printf("\n2. Print shared file\n");
        printf("3. Edit shared file\n");
        printf("\n0. Quit\n");
        printf("\nEnter choice:");
        scanf("%d",&choice);

        if (choice == 1)
        {
            send_message();
        }
        else if (choice == 2)
        {
            print_local_file(&sharedFile);
        }
        else if (choice == 3)
        {
            edit_local_file(&sharedFile);
        }
        else if (choice == 0)
        {
            break;
        }
        else
        {
            printf("\nWrong choice\n");
        }
    }

    if (choice == 0)
    {
        printf("\nExiting\n");
        close(server_fd);
        free(name);
    }

    return 0;
}


void send_message()
{
    int number_of_clients;
    printf("Enter the number of clients you want to send the message to: ");
    scanf("%d", &number_of_clients);

    int PORT_server[number_of_clients];
    printf("Enter the ports to send messages:\n");

    for (int i = 0; i < number_of_clients; i++)
    {
        scanf("%d", &PORT_server[i]);
    }

    int sockets[number_of_clients];

    for (int i = 0; i < number_of_clients; i++)
    {
        if ((sockets[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\nSocket creation error\n");
            return;
        }

        struct sockaddr_in serv_addr;
        memset(&serv_addr, '0', sizeof(serv_addr));

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(PORT_server[i]);

        if (connect(sockets[i], (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            printf("\nConnection Failed for port %d\n", PORT_server[i]);
            return;
        }
    }

    char* buffer;
    buffer=(char*)malloc(sizeof(char)*1024);
    printf("Enter your message: ");
    scanf(" %[^\n]s", buffer);

    for (int i = 0; i < number_of_clients; i++)
    {
        Message message;
        strcpy(message.name,name);
        message.port = PORT_server[i];
        strcpy(message.message, buffer);
        message.timestamp = logical_clock;
        message.file = 0;

        total_order_multicast(&message);
        logical_clock++;
    }
    free(buffer);
}


void *receive_thread(void *server_fd)
{
    int s_fd = *((int *)server_fd);

    while (1)
    {
        sleep(2);
        receive_message(s_fd);
    }
}


void total_order_multicast(Message *message)
{
    int server_fd, valread;
    struct sockaddr_in address;

    //send first to server to check timestamp
 
     if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(servers_port);

    if (connect(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        printf("\nConnection Failed for port %d\n",servers_port);
        return;
    }
    message->port_from = PORT;
    send(server_fd, message, sizeof(Message), 0);
    printf("Message sent to port %d\n", servers_port);
    
    
    
    Check check;
    memset(&check,0,sizeof(Check));

    sleep(2);
        valread = recv(server_fd, &check, sizeof(Check),0);
        close(server_fd);
    
   
    if(check.go == 1)
    {
        if(check.correct_timestamp != logical_clock)
        {   
            //testing gia an prokeitai gia diorthosi tou timestamp
            //printf("correct_timestamp from server %d\n",check.correct_timestamp);
            logical_clock = check.correct_timestamp;
        }

    //apostoli ston client
     if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(message->port);

    if (connect(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        printf("\nConnection Failed for port %d\n", message->port);
        return;
    }
    message->port_from = PORT;
    message->timestamp = logical_clock;
    send(server_fd, message, sizeof(Message), 0);
    printf("Message sent to port %d\n", message->port);
    close(server_fd);
}

}

void load_local_file(const char* filename, SharedFile* sharedFile) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open local file");
        exit(EXIT_FAILURE);
    }

    // Read the contents of the file into the shared file object
    size_t bytesRead = fread(sharedFile->content, sizeof(char), MAX_FILE_SIZE, file);
    if (bytesRead > 0) {
        sharedFile->content[bytesRead] = '\0'; // Null-terminate the content
    }

    fclose(file);
}

void print_local_file(SharedFile* shareFile)
{
    FILE* file =fopen(FILENAME,"r");
    if(file == NULL)
    {
        printf("error opening the file\n");
        return;
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer =(char*)malloc(file_size + 1);
    
    if (buffer == NULL) {
        printf("Error allocating memory.\n");
        fclose(file);
        return;
    }

    // Read the file contents into the buffer
    size_t elements_read = fread(buffer, sizeof(char), file_size, file);

    if (elements_read != file_size) {
        printf("Error reading the file.\n");
        free(buffer);
        fclose(file);
        return;
    }

    // Null-terminate the buffer
    buffer[file_size] = '\0';

    // Close the file
    fclose(file);

    // Print the file contents
    printf("%s\n", buffer);

    // Free the allocated memory
    free(buffer);
   
    return;
}

void edit_local_file(SharedFile *sharefile)
{
    system("vim shared_file.txt");
    load_local_file(FILENAME, sharefile);
    send_local_file(sharefile);
    return;
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

    while (1)
    {
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
                    if(message.file == 1)
                    {
                        printf("received file from port:%d\n",message.port);
                        FILE* file = fopen(FILENAME, "w");
                        if (file == NULL)
                        {
                            printf("Error opening the file\n");
                            break;
                        }
                         int index = 0;
                         while (message.message[index] != '\0') 
                         {
                         fwrite(&message.message[index], sizeof(char), 1, file);
                         index++;
                         }
                        fclose(file);
                        printf("changes saved to local copy \n");
                    }
                    else if(message.file == 0)
                    {
                        printf("\nUser:%s type:%d, timestamp:%d, port:%d says: %s\n",message.name, message.file, message.timestamp, message.port, message.message);
                    }
                    FD_CLR(i, &current_sockets);
                }
            }
        }

        if (k == (FD_SETSIZE * 2))
            break;
    }
}

void send_local_file (SharedFile *sharefile)
{
    int number_of_clients;
    printf("Enter the number of clients you want to send the message: ");
    scanf("%d", &number_of_clients);

    int PORT_server[number_of_clients];
    printf("Enter the ports to send messages:\n");

    for (int i = 0; i < number_of_clients; i++)
    {
        scanf("%d", &PORT_server[i]);
    }

    int sockets[number_of_clients];

    for (int i = 0; i < number_of_clients; i++)
    {
        if ((sockets[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\nSocket creation error\n");
            return;
        }

        struct sockaddr_in serv_addr;
        memset(&serv_addr, '0', sizeof(serv_addr));

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(PORT_server[i]);

        if (connect(sockets[i], (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            printf("\nConnection Failed for port %d\n", PORT_server[i]);
            return;
        }
    }

    char message[MAX_FILE_SIZE + 6];
    strcpy(message,sharefile->content);

    for (int i = 0; i < number_of_clients; i++)
    {
        Message fileMessage;
        strcpy(fileMessage.name,name);
        fileMessage.port = PORT_server[i];
        fileMessage.timestamp = logical_clock++;
        fileMessage.file = 1;
        strcpy(fileMessage.message, message);
        total_order_multicast(&fileMessage);
    }

    printf("File sent to specified ports.\n");
}
