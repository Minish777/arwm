#include <stdlib.h>
#include "arwm.h"

Monitor *monitors = NULL;

void monitor_init(void) {
    monitors = malloc(sizeof(Monitor));
    monitors->x = 0; monitors->y = 0;
    monitors->w = screen->width_in_pixels;
    monitors->h = screen->height_in_pixels;
    monitors->next = NULL;
}

void monitor_cleanup(void) {
    while (monitors) {
        Monitor *tmp = monitors;
        monitors = monitors->next;
        free(tmp);
    }
}
