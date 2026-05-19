#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"

int cfg_gap = 8;
float cfg_master_ratio = 0.6f;

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

    fprintf(f, "# ARWM Config\ngap 8\nmaster_ratio 0.6\n");
    fclose(f);
}

void config_load() {
    char file[512];
    get_config_path(file, sizeof(file));

    FILE *f = fopen(file, "r");
    if (!f) return;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        sscanf(line, "gap %d", &cfg_gap);
        sscanf(line, "master_ratio %f", &cfg_master_ratio);
    }
    fclose(f);
}
