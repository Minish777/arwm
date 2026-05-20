#ifndef CONFIG_H
#define CONFIG_H

#include <X11/Xlib.h>

void config_init();
void config_load();

extern int cfg_gap;
extern float cfg_master_ratio;
extern int cfg_border_width;
extern unsigned long cfg_border_active;
extern unsigned long cfg_border_inactive;

#endif
