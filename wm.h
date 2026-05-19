#ifndef WM_H
#define WM_H

#include <X11/Xlib.h>

extern Display *display;
extern Window root;
extern float camera_yaw;
extern float camera_pitch;
extern float radius;

typedef struct {
    Window client;
    Window frame;
    float angle;
    float target_angle;
    int dragging;
    int drag_start_x;
    int drag_start_y;
    int frame_x, frame_y;
} ManagedWindow;

void update_orbit_positions();
void on_window_map_request(XMapRequestEvent *ev);
void on_window_unmap(XUnmapEvent *ev);
void on_button_press(XButtonEvent *ev);
void on_button_release(XButtonEvent *ev);
void on_motion_notify(XMotionEvent *ev);
void on_key_press(XKeyEvent *ev);
void query_and_manage_existing_windows();
void register_global_hotkeys();
void style_frame_window(Window frame);
void calculate_window_position(float angle, float r, int *out_x, int *out_y);
void process_pending_events();
void set_cursor(Window win, int cursor_shape);
void clamp_window_position(int *x, int *y, int width, int height);

#endif