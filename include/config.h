#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

typedef struct {
    uint32_t gap_inner;
    uint32_t gap_outer;
    uint32_t border_width;
    uint32_t border_focus;
    uint32_t border_normal;
    float master_ratio;
    char terminal[64];
    char launcher[64];
} Config;

extern Config cfg;

void config_load_defaults(void);
void config_load(void);
void config_reload(void);

#endif
