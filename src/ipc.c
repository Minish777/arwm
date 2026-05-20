#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "arwm.h"

static int sock_fd = -1;

void ipc_init(void) {
    char path[256];
    const char *home = getenv("HOME");
    if (!home) return;

    char dir[256];
    snprintf(dir, sizeof(dir), "%s/.cache/arwm", home);
    mkdir(dir, 0755);

    snprintf(path, sizeof(path), "%s/ipc.sock", dir);
    unlink(path);

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) return;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);

    if (bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        close(sock_fd);
        sock_fd = -1;
        return;
    }
    listen(sock_fd, 5);
    fcntl(sock_fd, F_SETFL, O_NONBLOCK);
}

void ipc_cleanup(void) {
    if (sock_fd != -1) {
        char path[256];
        snprintf(path, sizeof(path), "%s/.cache/arwm/ipc.sock", getenv("HOME"));
        unlink(path);
        close(sock_fd);
    }
}

void ipc_handle_clients(void) {
    if (sock_fd == -1) return;
    int client_fd = accept(sock_fd, NULL, NULL);
    if (client_fd == -1) return;

    char buf[128];
    ssize_t len = read(client_fd, buf, sizeof(buf)-1);
    if (len > 0) {
        buf[len] = '\0';
        if (strcmp(buf, "quit") == 0) exit(0);
        else if (strcmp(buf, "reload") == 0) config_reload();
        else if (strncmp(buf, "spawn ", 6) == 0) spawn(buf + 6);
    }
    close(client_fd);
}
