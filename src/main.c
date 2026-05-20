#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <poll.h>
#include "arwm.h"

xcb_connection_t *conn;
xcb_screen_t *screen;
xcb_ewmh_connection_t ewmh;

static void handle_signal(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        arwm_cleanup();
        exit(0);
    }
}

int main(void) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    arwm_init();

    log_info("ARWM started successfully.");

    spawn("alacritty || xterm");
    spawn("sh ~/.config/arwm/autostart.sh");

    int x11_fd = xcb_get_file_descriptor(conn);
    struct pollfd fds[1];
    fds[0].fd = x11_fd;
    fds[0].events = POLLIN;

    while (1) {
        xcb_generic_event_t *ev;
        while ((ev = xcb_poll_for_event(conn))) {
            events_handle(ev);
            free(ev);
        }

        ipc_handle_clients();

        if (poll(fds, 1, 10) > 0) {
            if (fds[0].revents & POLLIN) {
                // Wake up to process events
            }
        }
    }

    arwm_cleanup();
    return 0;
}
