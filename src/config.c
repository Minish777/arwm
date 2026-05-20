#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "config.h"
#include "util.h"

Config cfg;

void config_load_defaults(void) {
    cfg.gap_inner = 10;
    cfg.gap_outer = 10;
    cfg.border_width = 2;
    cfg.border_focus = 0x61AFEF;
    cfg.border_normal = 0x3E4452;
    cfg.master_ratio = 0.55f;
    strncpy(cfg.terminal, "alacritty", 63);
    strncpy(cfg.launcher, "dmenu_run", 63);
}

void config_load(void) {
    char path[256];
    const char *home = getenv("HOME");
    if (!home) return;

    char dir[256];
    snprintf(dir, sizeof(dir), "%s/.config/arwm", home);
    mkdir(dir, 0755);

    snprintf(path, sizeof(path), "%s/config.conf", dir);

    FILE *f = fopen(path, "r");
    if (!f) {
        f = fopen(path, "w");
        if (f) {
            fprintf(f, "gap_inner 10\ngap_outer 10\nterminal alacritty\n");
            fclose(f);
        }
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char key[64], val[64];
        if (sscanf(line, "%s %s", key, val) != 2) continue;
        if (strcmp(key, "gap_inner") == 0) cfg.gap_inner = atoi(val);
        else if (strcmp(key, "gap_outer") == 0) cfg.gap_outer = atoi(val);
        else if (strcmp(key, "terminal") == 0) strncpy(cfg.terminal, val, 63);
    }
    fclose(f);
}

void config_reload(void) {
    log_info("Reloading configuration...");
    config_load();
    layout_apply();
}
