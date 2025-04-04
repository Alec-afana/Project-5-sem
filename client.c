#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>  // Для измерения времени
#include <stdint.h>
#include <endian.h>

#define SERVER_ADDR "example.com"
#define SERVER_PORT 8080
#define CLIENT_REQUEST "Hello world!"

#define BUFFER_LEN 8192 //1024

// Функция для преобразования 64-битного числа из сетевого порядка в порядок хоста
uint64_t ntohll(uint64_t value) {
    uint32_t high_part = ntohl((uint32_t)(value >> 32)); // Старшие 32 бита
    uint32_t low_part = ntohl((uint32_t)(value & 0xFFFFFFFF)); // Младшие 32 бита
    return ((uint64_t)low_part << 32) | high_part;
}


int main() 
{
    struct timeval start, end;
    long seconds, microseconds;
    double elapsed;

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

    // Замер времени начала передачи
    //gettimeofday(&start, NULL);

    // Получение ответа от сервера
    FILE *file = fopen("received_file.txt", "wb");// Записываем файл в двоичном режиме
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

    //off_t file_size;
    //recv(client_socket, &file_size, sizeof(file_size), 0);


    // Получение размера файла
    uint64_t net_file_size;
    recv(client_socket, &net_file_size, sizeof(net_file_size), 0);
    uint64_t file_size = ntohll(net_file_size); // Преобразуем в порядок хоста


	//time start
    gettimeofday(&start, NULL);

    //ssize_t bytes_received;
    off_t bytes_received = 0;
	//в этом случае получаем размер файла заранее и считываем по размеру
    while (bytes_received < file_size) {
	ssize_t len = recv(client_socket, buffer, BUFFER_LEN, 0);
	if (len <= 0) break;
	fwrite(buffer, 1, len, file);
	bytes_received += len;
    }

    //printf("Server response: ");
/*
    ssize_t bytes_received;
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
*/
    fclose(file);
    //printf("File received successfully!\n");

    shutdown (client_socket, SHUT_WR);

    // Замер времени окончания передачи
    gettimeofday(&end, NULL);

    // Рассчёт времени передачи


    seconds = end.tv_sec - start.tv_sec;
    microseconds = end.tv_usec - start.tv_usec;
    elapsed = seconds + microseconds*1e-6;

    printf("Time taken to receive file: %.3f seconds.\n", elapsed);
    

    double speed = file_size / elapsed;
    printf("Time: %.3f seconds, Speed: %.3f bytes/second\n", elapsed, speed);
    printf("Speed: %.2f bytes/second\n", file_size / elapsed);
    printf("Speed: %.2f KB/second\n", (file_size / 1024.0) / elapsed);
    printf("Speed: %.2f MB/second\n", (file_size / (1024.0 * 1024.0)) / elapsed);

/*
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    double speed = file_size / elapsed;
    printf("Time: %.2f seconds, Speed: %.2f bytes/second\n", elapsed, speed);
*/

    // Отправка команды завершения
/*
    send(client_socket, "quit\n", strlen("quit\n"), 0);
    
    if (bytes_received < 0)
        perror ("Server response error");
    else
        printf("Server response end");

    close(client_socket);
*/
    return 0;
}
