#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "config.h"

int cfg_gap = 8;
float cfg_master_ratio = 0.6f;

void config_init() {

    char *home = getenv("HOME");

    char dir[512];
    snprintf(dir, sizeof(dir),
        "%s/.config/arwm", home);

    mkdir(dir, 0755);

    char file[600];
    snprintf(file, sizeof(file),
        "%s/config.conf", dir);

    FILE *f = fopen(file, "r");
    if (f) {
        fclose(f);
        return;
    }

    f = fopen(file, "w");
    if (!f) return;

    fprintf(f,
"# ARWM live config\n"
"mod SUPER\n\n"
"bind ENTER spawn alacritty\n"
"bind D spawn dmenu_run\n"
"bind B spawn firefox\n\n"
"bind H split_vertical\n"
"bind V split_horizontal\n\n"
"bind Q quit\n"
"bind R reload\n\n"
"gap 8\n"
"master_ratio 0.60\n"
);

    fclose(f);
}

void config_load() {

    char file[600];
    snprintf(file, sizeof(file),
        "%s/.config/arwm/config.conf",
        getenv("HOME"));

    FILE *f = fopen(file, "r");
    if (!f) return;

    char line[256];

    while (fgets(line, sizeof(line), f)) {

        sscanf(line, "gap %d", &cfg_gap);
        sscanf(line, "master_ratio %f", &cfg_master_ratio);
    }

    fclose(f);
}