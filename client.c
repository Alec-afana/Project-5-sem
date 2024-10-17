#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVER_ADDR "example.com"
#define SERVER_PORT 8080
#define CLIENT_REQUEST "Hello world!"

#define BUFFER_LEN 1024

int main() 
{

    struct hostent *server_info = gethostbyname(SERVER_ADDR);
    if (!server_info) 
    {
        perror("DNS Resolve error");
        exit(1);
    }
    
    char server_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, server_info->h_addr, server_ip, sizeof(server_ip));
    printf("Connecting to: %s\n", server_ip);

    
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) 
    {
        perror("Socket create");
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr = *((struct in_addr *)server_info->h_addr);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connect error");
        exit(1);
    }


    char filename[BUFFER_LEN];
    char buffer[BUFFER_LEN];

    // Ввод имени файла для запроса
    printf("Enter the name of the file you want to request: ");
    scanf("%s", filename);

    // Формирование запроса get_file
    snprintf(buffer, sizeof(buffer), "get_file %s", filename);
    //send(client_socket, buffer, strlen(buffer), 0);
    if (send(client_socket, buffer, strlen(buffer), 0) < 0)
    {
        perror ("Server request error");
        exit(1);
    }


    // Получение ответа от сервера
    FILE *file = fopen("received_file.txt", "w");
    if (!file) {
        perror("File creation failed");
        exit(EXIT_FAILURE);
    }

/*
    if (send(client_socket, CLIENT_REQUEST, strlen(CLIENT_REQUEST), 0) < 0)
    {
        perror ("Server request error");
        exit(1);
    }
    shutdown (client_socket, SHUT_WR);
*/
    //char filename[BUFFER_LEN];

    //char buffer[BUFFER_LEN];
    ssize_t bytes_received;
    
    //printf("Server response: ");
    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytes_received] = '\0';
	if (strcmp(buffer, "File not found\n") == 0) {
            printf("Error: File not found on server.\n");
            break;
        }
        fwrite(buffer, sizeof(char), bytes_received, file);  // Запись данных в файл
        //printf("'%s'\n", buffer);
    }

    fclose(file);
    //printf("File received successfully!\n");

    // Отправка команды завершения
    send(client_socket, "quit\n", strlen("quit\n"), 0);
    
    if (bytes_received < 0)
        perror ("Server response error");
    else
        printf("Server response end");

    close(client_socket);

    return 0;
}
