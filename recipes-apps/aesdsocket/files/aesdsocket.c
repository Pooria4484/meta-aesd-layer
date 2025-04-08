#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifndef USE_AESD_CHAR_DEVICE
#define USE_AESD_CHAR_DEVICE 1
#endif

#if USE_AESD_CHAR_DEVICE
    const char *file_path = "/dev/aesdchar";
#else
    const char *file_path = "/var/tmp/aesdsocketdata";
#endif

#define PORT 9000
#define BUFFER_SIZE 1024

static volatile sig_atomic_t keep_running = 1;

void signal_handler(int sig) {
    keep_running = 0;
    syslog(LOG_INFO, "Caught signal, exiting");
}

int daemonize() {
    pid_t pid = fork();
    
    if (pid < 0) {
        syslog(LOG_ERR, "Fork failed: %s", strerror(errno));
        return -1;
    }
    
    if (pid > 0) {
        exit(0);
    }
    
    if (setsid() < 0) {
        syslog(LOG_ERR, "setsid failed: %s", strerror(errno));
        return -1;
    }
    
    if (chdir("/") < 0) {
        syslog(LOG_ERR, "chdir failed: %s", strerror(errno));
        return -1;
    }
    
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    int fd = open("/dev/null", O_RDWR);
    if (fd != -1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > 2) close(fd);
    }
    
    return 0;
}

int main(int argc, char *argv[]) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char client_ip[INET_ADDRSTRLEN];
    int daemon_mode = 0;
    
    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        daemon_mode = 1;
    }
    
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    openlog("aesdsocket", LOG_PID, LOG_USER);
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        syslog(LOG_ERR, "Socket creation failed: %s", strerror(errno));
        return -1;
    }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        syslog(LOG_ERR, "Bind failed: %s", strerror(errno));
        close(server_fd);
        return -1;
    }
    
    if (listen(server_fd, 5) == -1) {
        syslog(LOG_ERR, "Listen failed: %s", strerror(errno));
        close(server_fd);
        return -1;
    }
    
    if (daemon_mode) {
        if (daemonize() < 0) {
            close(server_fd);
            return -1;
        }
    }
    
    while (keep_running) {
        fd_set readfds;
        struct timeval tv = {.tv_sec = 1, .tv_usec = 0};
        
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        
        int ready = select(server_fd + 1, &readfds, NULL, NULL, &tv);
        
        if (!keep_running) break;
        if (ready <= 0) continue;
        
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            syslog(LOG_ERR, "Accept failed: %s", strerror(errno));
            continue;
        }
        
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        syslog(LOG_INFO, "Accepted connection from %s", client_ip);
        
        int data_fd = open(file_path, O_RDWR | O_CREAT, 0644);
        if (data_fd == -1) {
            syslog(LOG_ERR, "Failed to open device file: %s", strerror(errno));
            close(client_fd);
            continue;
        }
        
        ssize_t bytes_received;
        while ((bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0) {
            ssize_t total_written = 0;
            while (total_written < bytes_received) {
                ssize_t w = write(data_fd, buffer + total_written, bytes_received - total_written);
                if (w < 0) {
                    syslog(LOG_ERR, "write failed: %s", strerror(errno));
                    break;
                }
                total_written += w;
            }

            lseek(data_fd, 0, SEEK_SET);
            ssize_t r;
            while ((r = read(data_fd, buffer, BUFFER_SIZE)) > 0) {
                send(client_fd, buffer, r, 0);
            }
        }
        
        close(data_fd);
        close(client_fd);
        syslog(LOG_INFO, "Closed connection from %s", client_ip);
    }
    
    close(server_fd);
#if !USE_AESD_CHAR_DEVICE
    remove(file_path);
#endif
    closelog();
    
    return 0;
}
