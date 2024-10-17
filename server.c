#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <errno.h>

/* Configuration */
#define SERVER_ADDR      inet_addr("127.0.0.1"); //INADDR_ANY // inet_addr("192.168.0.1");
#define SERVER_PORT      8080
#define SERVER_MAX_CONN  5
#define LOG_FILE         "log.txt"
#define LOG_LEVEL        0
#define CLIENT_TIMEOUT   10

/* Example */
#define BUFFER_LEN       1024
#define FILE_DIR	"/home/alec/server_files/" //каталог файлов

/* Macros */
#define LOG_DBG          0
#define LOG_MSG          1
#define LOG_WRN          2
#define LOG_ERR          3

#define SERVER_GREETING  "Greetings from Server!\n"

void pr_log(int level, const char *format, ...) 
{
    if (level >= LOG_LEVEL)
    {
        FILE *log = fopen (LOG_FILE, "a");
        if (!log) return;
        
        time_t now = time(NULL);
        struct tm local_time;
        localtime_r(&now, &local_time);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S %z", &local_time);        
        fprintf(log, "[PID=%u %s] ", getpid(), timestamp);
    
        va_list args;
        va_start(args, format);
        vfprintf(log, format, args);
        va_end(args);
        
        fprintf(log, "\n");

        fclose(log);        
    }
}


volatile sig_atomic_t server_running = 1;

void signal_handler(int signo) 
{
    pr_log (LOG_WRN, "Signal recived: %u", signo);
    if (signo == SIGTERM || signo == SIGINT) server_running = 0;
}

void daemonize() 
{
    pid_t pid, sid;

    pid = fork();
    if (pid < 0) 
    {
        perror ("Fork 1 failure");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) 
    {
        exit(EXIT_SUCCESS);
    }    
    pr_log (LOG_DBG, "Fork 1");

    umask(0);

    sid = setsid();
    if (sid < 0) 
    {
        perror ("Setsid failure");
        exit(EXIT_FAILURE);
    }
    pr_log (LOG_DBG, "Setsid");
    
    pid = fork();
    if (pid < 0) 
    {
        perror ("Fork 2 failure");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) 
    {
        exit(EXIT_SUCCESS);
    }
    pr_log (LOG_DBG, "Fork 2");

/*
    if ((chdir("/")) < 0) 
    {
        perror ("Chdir failure");
        exit(EXIT_FAILURE);
    }
    pr_log (LOG_DBG, "Chdir");
*/

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    open("/dev/null", O_RDWR);
    dup(STDIN_FILENO);
    dup(STDIN_FILENO);

    pr_log (LOG_MSG, "Demon started with PID=%u", getpid());

}

void *handle_client(void *client_socket) 
{
    int client_sock = *((int *)client_socket);
    
    char buffer[BUFFER_LEN];
    ssize_t bytes_received;

    while (server_running) 
    {
    
        bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received < 0) 
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN) 
            {
                pr_log(LOG_WRN, "Client timeout reached");
                break;
            }
            pr_log(LOG_ERR, "Recv error");
            break;
        }
        
        if (bytes_received == 0) 
        {
            pr_log(LOG_MSG, "Client data finished");
            break;
        }

        buffer[bytes_received] = '\0';
        pr_log(LOG_DBG, "Received from client: '%s'", buffer);

	//проверяем запросили ли файл
	if (strncmp(buffer, "get_file", 8) == 0) {
		char *filename = strtok(buffer + 9, " "); //извлекаем имя файла
		//надо проверить правильно ли имя файла получаем
		pr_log(LOG_DBG, "Filename requested: %s", filename);//отладка имени файла
		char filepath[BUFFER_LEN];
		snprintf(filepath, sizeof(filepath), "%s%s", FILE_DIR, filename); //путь к файлу
		pr_log(LOG_DBG, "Filepath: %s", filepath);//отладка пути

		FILE *file = fopen(filepath, "r");
		if (!file) {
			//отправляем сообщение если файл не найден
			char *error_message = "File not found\n";
			send(client_sock, error_message, strlen(error_message), 0);
			perror("File not found");
		} else {
			pr_log(LOG_DBG, "Sending file: %s\n", filepath);//отладка отправки файла
			//передаем файл клиенту
			while (fgets(buffer, BUFFER_LEN, file) != NULL) {
				send(client_sock, buffer, strlen(buffer), 0);
			}
			fclose(file);
			pr_log (LOG_MSG, "File sent successfully!!!!!!!!!!");
		}
	}

	//ожидание команды завершения
	recv(client_sock, buffer, BUFFER_LEN, 0);
		//а надо ли  здесь \n после quit? yes
	if (strcmp(buffer, "quit\n") == 0) {
		pr_log (LOG_MSG, "Client requested to close connection\n");
		close(client_sock);
	}

        //send(client_sock, buffer, bytes_received, 0);        
        //pr_log (LOG_MSG, "Respose sent");

    }
    
    close(client_sock);
    pr_log (LOG_MSG, "Connection closed");
    
    pthread_detach(pthread_self());
    pthread_exit(NULL);
}

int main() {

    if (signal(SIGTERM, signal_handler) == SIG_ERR)
    {
        perror("SIGTERM");
        exit (EXIT_FAILURE);
    }
    if (signal(SIGINT, signal_handler) == SIG_ERR)
    {
        perror("SIGINT");
        exit (EXIT_FAILURE);
    }

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) 
    {
        perror("Socket create");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = SERVER_ADDR;
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
    {
        perror("Socket bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, SERVER_MAX_CONN) < 0) 
    {
        perror("Socket listen");
        exit(EXIT_FAILURE);
    }
    
    FILE *log = fopen (LOG_FILE, "w");
    if (!log)
    {
        perror("Log file");
        exit(EXIT_FAILURE);
    }
    fclose (log);

    daemonize();

    while (server_running) 
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);

        struct timeval timeout;
        timeout.tv_usec = 0;
        timeout.tv_sec = 1;

        int ready = select(server_socket + 1, &readfds, NULL, NULL, &timeout);
        if (ready < 0) 
        {
            if (!server_running) break;
            pr_log(LOG_ERR, "Select error");
            server_running = 0;
            break;
            
        } 
        
        if (ready == 0) continue;
        
        if (FD_ISSET(server_socket, &readfds)) 
        {
            int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);

            if (client_socket == -1)
            {
                if (!server_running) break;    
                pr_log (LOG_ERR, "Socket accept error");
                continue;
            }
        
            if (getpeername(client_socket, (struct sockaddr *)&client_addr, &client_len) == 0) 
            {
                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
                int client_port = ntohs(client_addr.sin_port);
                pr_log (LOG_MSG, "Accepted connection from %s:%d", client_ip, client_port);
            }
        
        
            struct timeval timeout;      
            timeout.tv_sec = CLIENT_TIMEOUT;
            timeout.tv_usec = 0;
    
            if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0)
            {
                pr_log (LOG_ERR, "Set timeout error: '%s'", strerror(errno));
                close(client_socket);
                break;
            }

            //send(client_socket, SERVER_GREETING, strlen (SERVER_GREETING), 0);
            
            pthread_t client_thread;
            if (pthread_create(&client_thread, NULL, handle_client, &client_socket) != 0) 
            {
                pr_log (LOG_ERR, "Client pthread starting error");
                close(client_socket);
            }
        }
    }

    close(server_socket);

    pr_log (LOG_MSG, "Finished");

    return 0;
}
