#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <X11/Xlib.h>

#include "config.h"

int cfg_gap = 8;
float cfg_master_ratio = 0.6f;
int cfg_border_width = 2;
unsigned long cfg_border_active = 0x4444ff;   // Blue-ish
unsigned long cfg_border_inactive = 0x222222; // Dark gray

static void get_config_path(char *path, size_t len) {
    char *home = getenv("HOME");
    if (!home) home = "/root";
    snprintf(path, len, "%s/.config/arwm/config.conf", home);
}

void config_init() {
    char dir[512];
    char *home = getenv("HOME");
    if (!home) home = "/root";

    snprintf(dir, sizeof(dir), "%s/.config", home);
    mkdir(dir, 0755);
    snprintf(dir, sizeof(dir), "%s/.config/arwm", home);
    mkdir(dir, 0755);

    char file[512];
    get_config_path(file, sizeof(file));

    if (access(file, F_OK) == 0) return;

    FILE *f = fopen(file, "w");
    if (!f) return;

    fprintf(f, "# ARWM Config\ngap 8\nmaster_ratio 0.6\nborder_width 2\nborder_active 0x4444ff\nborder_inactive 0x222222\n");
    fclose(f);
}

static unsigned long parse_color(const char *hex) {
    return strtoul(hex, NULL, 16);
}

void config_load() {
    char file[512];
    get_config_path(file, sizeof(file));

    FILE *f = fopen(file, "r");
    if (!f) return;

    char line[256];
    char key[64], val[128];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        if (sscanf(line, "%s %s", key, val) == 2) {
            if (strcmp(key, "gap") == 0) cfg_gap = atoi(val);
            else if (strcmp(key, "master_ratio") == 0) cfg_master_ratio = atof(val);
            else if (strcmp(key, "border_width") == 0) cfg_border_width = atoi(val);
            else if (strcmp(key, "border_active") == 0) cfg_border_active = parse_color(val);
            else if (strcmp(key, "border_inactive") == 0) cfg_border_inactive = parse_color(val);
        }
    }
    fclose(f);
}
