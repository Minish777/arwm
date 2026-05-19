#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <setjmp.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include "wm.h"

/* Global display state. */
Display *display;
Window root;
float camera_yaw = 0.0f;
float camera_pitch = 0.0f;
float radius = 250.0f;

ManagedWindow managed_windows[100];
int window_count = 0;

int screen_center_x = 400;
int screen_center_y = 300;
int screen_width = 800;
int screen_height = 600;

typedef struct {
    int is_orbiting;
    int is_dragging_window;
    int orbit_start_x;
    int orbit_start_y;
    float orbit_yaw_start;
    float orbit_pitch_start;
    Window dragged_window;
} InputState;

InputState input_state = {0};

static jmp_buf io_error_jmp;
static volatile int x_error_occurred = 0;

static int x_error_handler(Display *display_arg, XErrorEvent *error_event) {
    char buffer[256];
    XGetErrorText(display_arg, error_event->error_code, buffer, sizeof(buffer));
    fprintf(stderr, "[XError] request=%d error=%d (%s)
",
            error_event->request_code,
            error_event->error_code,
            buffer);
    x_error_occurred = error_event->error_code;
    return 0;
}

static int x_io_error_handler(Display *display_arg) {
    fprintf(stderr, "[XIOError] fatal I/O error on display %s
", DisplayString(display_arg));
    siglongjmp(io_error_jmp, 1);
}

static void sync_display(void) {
    if (display != NULL) {
        XSync(display, False);
    }
}

static int window_exists(Window window) {
    XWindowAttributes attrs;
    return window != None && XGetWindowAttributes(display, window, &attrs) != 0;
}

static int window_is_managed(Window window) {
    for (int i = 0; i < window_count; i++) {
        if (managed_windows[i].frame == window || managed_windows[i].client == window) {
            return 1;
        }
    }
    return 0;
}

void set_cursor(Window win, int cursor_shape) {
    Cursor cursor = XCreateFontCursor(display, cursor_shape);
    if (cursor != None) {
        XDefineCursor(display, win, cursor);
        XFreeCursor(display, cursor);
    }
}

void clamp_window_position(int *x, int *y, int width, int height) {
    int min_x = 10;
    int max_x = screen_width - width - 10;
    int min_y = 10;
    int max_y = screen_height - height - 10;

    if (*x < min_x) {
        *x = min_x;
    }
    if (*x > max_x) {
        *x = max_x;
    }
    if (*y < min_y) {
        *y = min_y;
    }
    if (*y > max_y) {
        *y = max_y;
    }
}

void style_frame_window(Window frame) {
    XSetWindowBorderWidth(display, frame, 2);
    XSetWindowBorder(display, frame, 0x0000FF);
    XSetWindowBackground(display, frame, 0x222222);
}

void calculate_window_position(float angle, float r, int *out_x, int *out_y) {
    float absolute_angle = angle + camera_yaw;
    float radius_factor = 1.0f + (camera_pitch * 0.1f);
    float effective_radius = r * radius_factor;

    int x = screen_center_x + (int)(effective_radius * sinf(absolute_angle)) - 200;
    int y = screen_center_y + (int)(effective_radius * cosf(absolute_angle)) - 150;

    *out_x = x;
    *out_y = y;
}

void update_orbit_positions(void) {
    for (int i = 0; i < window_count; i++) {
        if (managed_windows[i].dragging) {
            continue;
        }

        if (!window_exists(managed_windows[i].frame)) {
            continue;
        }

        int x;
        int y;
        calculate_window_position(managed_windows[i].angle, radius, &x, &y);
        XMoveWindow(display, managed_windows[i].frame, x, y);
    }

    sync_display();
}

static void remove_managed_window(int index) {
    if (index < 0 || index >= window_count) {
        return;
    }

    if (window_exists(managed_windows[index].frame)) {
        XDestroyWindow(display, managed_windows[index].frame);
        sync_display();
    }

    for (int j = index; j < window_count - 1; j++) {
        managed_windows[j] = managed_windows[j + 1];
    }
    window_count--;
}

void on_window_map_request(XMapRequestEvent *ev) {
    if (!window_exists(ev->window)) {
        return;
    }

    XWindowAttributes attrs;
    if (!XGetWindowAttributes(display, ev->window, &attrs)) {
        fprintf(stderr, "[MapRequest] XGetWindowAttributes failed for %lu
", ev->window);
        sync_display();
        return;
    }

    Window frame = XCreateSimpleWindow(display, root, 0, 0,
                                      attrs.width, attrs.height, 2,
                                      0x0000FF, 0x222222);
    if (frame == 0) {
        fprintf(stderr, "[MapRequest] Failed to create frame window
");
        sync_display();
        return;
    }

    style_frame_window(frame);
    if (attrs.width > 0 && attrs.height > 0) {
        XResizeWindow(display, frame, attrs.width, attrs.height);
    }
    sync_display();

    XSelectInput(display, frame,
                 EnterWindowMask | LeaveWindowMask | ExposureMask |
                 ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
                 PointerMotionMask);
    XSelectInput(display, ev->window,
                 StructureNotifyMask | PropertyChangeMask | FocusChangeMask);
    sync_display();

    if (!window_exists(ev->window)) {
        XDestroyWindow(display, frame);
        sync_display();
        return;
    }

    if (XReparentWindow(display, ev->window, frame, 0, 0) == 0) {
        fprintf(stderr, "[MapRequest] XReparentWindow failed for %lu
", ev->window);
        XDestroyWindow(display, frame);
        sync_display();
        return;
    }
    sync_display();

    if (window_exists(frame)) {
        XMapWindow(display, frame);
        sync_display();
    }
    if (window_exists(ev->window)) {
        XMapWindow(display, ev->window);
        sync_display();
    }

    if (window_exists(ev->window)) {
        XSetInputFocus(display, ev->window, RevertToPointerRoot, CurrentTime);
        sync_display();
    }

    if (window_count < 100) {
        ManagedWindow *mgd = &managed_windows[window_count];
        mgd->client = ev->window;
        mgd->frame = frame;
        mgd->angle = (2.0f * M_PI * window_count) / 100.0f;
        mgd->target_angle = mgd->angle;
        mgd->dragging = 0;
        mgd->drag_start_x = 0;
        mgd->drag_start_y = 0;
        mgd->frame_x = 0;
        mgd->frame_y = 0;

        window_count++;
        update_orbit_positions();
    }
}

void on_window_unmap(XUnmapEvent *ev) {
    for (int i = 0; i < window_count; i++) {
        if (managed_windows[i].client == ev->window) {
            remove_managed_window(i);
            update_orbit_positions();
            return;
        }
    }
}

void on_button_press(XButtonEvent *ev) {
    if (ev->button != Button3) {
        return;
    }

    int super_held = (ev->state & Mod4Mask) != 0;

    if (super_held) {
        ManagedWindow *mgd_win = NULL;
        for (int i = 0; i < window_count; i++) {
            if (managed_windows[i].frame == ev->window || managed_windows[i].client == ev->window) {
                mgd_win = &managed_windows[i];
                break;
            }
        }
        if (mgd_win == NULL) {
            return;
        }

        int grab_status = XGrabPointer(display, ev->window, False,
                                       ButtonMotionMask | ButtonReleaseMask,
                                       GrabModeAsync, GrabModeAsync,
                                       None, None, CurrentTime);
        if (grab_status != GrabSuccess) {
            fprintf(stderr, "[Input] XGrabPointer failed: %d
", grab_status);
            sync_display();
            return;
        }

        set_cursor(root, XC_fleur);
        input_state.is_dragging_window = 1;
        input_state.dragged_window = mgd_win->frame;
        mgd_win->dragging = 1;
        mgd_win->drag_start_x = ev->x_root;
        mgd_win->drag_start_y = ev->y_root;

        Window dummy_root;
        unsigned int width;
        unsigned int height;
        unsigned int border_width;
        unsigned int depth;
        XGetGeometry(display, mgd_win->frame, &dummy_root,
                     &mgd_win->frame_x, &mgd_win->frame_y,
                     &width, &height, &border_width, &depth);
        sync_display();
    } else {
        input_state.is_orbiting = 1;
        input_state.orbit_start_x = ev->x_root;
        input_state.orbit_start_y = ev->y_root;
        input_state.orbit_yaw_start = camera_yaw;
        input_state.orbit_pitch_start = camera_pitch;
        set_cursor(root, XC_circle);

        int grab_status = XGrabPointer(display, root, False,
                                       ButtonMotionMask | ButtonReleaseMask,
                                       GrabModeAsync, GrabModeAsync,
                                       None, None, CurrentTime);
        if (grab_status != GrabSuccess) {
            fprintf(stderr, "[Input] XGrabPointer failed for orbit: %d
", grab_status);
            sync_display();
            input_state.is_orbiting = 0;
            set_cursor(root, XC_arrow);
            return;
        }
        sync_display();
    }
}

void on_motion_notify(XMotionEvent *ev) {
    if (input_state.is_orbiting) {
        int dx = ev->x_root - input_state.orbit_start_x;
        int dy = ev->y_root - input_state.orbit_start_y;
        camera_yaw = input_state.orbit_yaw_start - (float)dx * 0.01f;
        camera_pitch = input_state.orbit_pitch_start + (float)dy * 0.01f;
        if (camera_pitch > M_PI / 2.0f) {
            camera_pitch = M_PI / 2.0f;
        }
        if (camera_pitch < -M_PI / 2.0f) {
            camera_pitch = -M_PI / 2.0f;
        }
        return;
    }

    if (!input_state.is_dragging_window) {
        return;
    }

    ManagedWindow *mgd_win = NULL;
    for (int i = 0; i < window_count; i++) {
        if (managed_windows[i].frame == input_state.dragged_window) {
            mgd_win = &managed_windows[i];
            break;
        }
    }
    if (mgd_win == NULL) {
        return;
    }

    int dx = ev->x_root - mgd_win->drag_start_x;
    int dy = ev->y_root - mgd_win->drag_start_y;
    int new_x = mgd_win->frame_x + dx;
    int new_y = mgd_win->frame_y + dy;

    Window dummy_root;
    int dummy_x;
    int dummy_y;
    unsigned int width;
    unsigned int height;
    unsigned int border_width;
    unsigned int depth;
    XGetGeometry(display, mgd_win->frame, &dummy_root, &dummy_x, &dummy_y,
                 &width, &height, &border_width, &depth);
    clamp_window_position(&new_x, &new_y, (int)width, (int)height);

    if (window_exists(mgd_win->frame)) {
        XMoveWindow(display, mgd_win->frame, new_x, new_y);
        sync_display();
    }
}

void on_button_release(XButtonEvent *ev) {
    if (ev->button != Button3) {
        return;
    }

    if (input_state.is_orbiting) {
        input_state.is_orbiting = 0;
    }
    if (input_state.is_dragging_window) {
        for (int i = 0; i < window_count; i++) {
            if (managed_windows[i].frame == input_state.dragged_window) {
                managed_windows[i].dragging = 0;
                break;
            }
        }
        input_state.is_dragging_window = 0;
        input_state.dragged_window = None;
    }

    XUngrabPointer(display, CurrentTime);
    set_cursor(root, XC_arrow);
    sync_display();
}

void on_key_press(XKeyEvent *ev) {
    KeyCode key_q = XKeysymToKeycode(display, XK_q);
    KeyCode key_e = XKeysymToKeycode(display, XK_e);
    KeyCode key_r = XKeysymToKeycode(display, XK_r);

    if ((ev->state & Mod4Mask) == 0) {
        return;
    }
    if (ev->keycode == key_e) {
        camera_yaw = 0.0f;
        camera_pitch = 0.0f;
    } else if (ev->keycode == key_r) {
        for (int i = 0; i < window_count; i++) {
            managed_windows[i].dragging = 0;
            managed_windows[i].angle = (2.0f * M_PI * i) / (float)window_count;
        }
    } else if (ev->keycode == key_q) {
        /* TODO: close focused window */
    }
}

void query_and_manage_existing_windows(void) {
    Window root_ret;
    Window parent_ret;
    Window *children = None;
    unsigned int nchildren = 0;

    if (!XQueryTree(display, root, &root_ret, &parent_ret, &children, &nchildren)) {
        return;
    }

    for (unsigned int i = 0; i < nchildren; i++) {
        XWindowAttributes attrs;
        if (!XGetWindowAttributes(display, children[i], &attrs)) {
            continue;
        }
        if (attrs.map_state == IsViewable) {
            XMapRequestEvent ev;
            memset(&ev, 0, sizeof(ev));
            ev.window = children[i];
            on_window_map_request(&ev);
        }
    }

    if (children != None) {
        XFree(children);
    }
}

void register_global_hotkeys(void) {
    KeyCode key_q = XKeysymToKeycode(display, XK_q);
    KeyCode key_e = XKeysymToKeycode(display, XK_e);
    KeyCode key_r = XKeysymToKeycode(display, XK_r);

    XGrabKey(display, key_q, Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(display, key_e, Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(display, key_r, Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);
    sync_display();
}

void process_pending_events(void) {
    while (XPending(display) > 0) {
        XEvent event_data;
        XNextEvent(display, &event_data);

        switch (event_data.type) {
            case MapRequest:
                on_window_map_request(&event_data.xmaprequest);
                break;
            case UnmapNotify:
                on_window_unmap(&event_data.xunmap);
                break;
            case ButtonPress:
                on_button_press(&event_data.xbutton);
                break;
            case ButtonRelease:
                on_button_release(&event_data.xbutton);
                break;
            case MotionNotify:
                on_motion_notify(&event_data.xmotion);
                break;
            case KeyPress:
                on_key_press(&event_data.xkey);
                break;
            default:
                break;
        }
    }
}

int main(void) {
    XSetErrorHandler(x_error_handler);
    XSetIOErrorHandler(x_io_error_handler);

    if (sigsetjmp(io_error_jmp, 1) != 0) {
        fprintf(stderr, "[Fatal] Recovered from X I/O error. Exiting.
");
        return 1;
    }

    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "[Fatal] Cannot open X display
");
        return 1;
    }

    root = RootWindow(display, DefaultScreen(display));
    screen_width = DisplayWidth(display, DefaultScreen(display));
    screen_height = DisplayHeight(display, DefaultScreen(display));
    screen_center_x = screen_width / 2;
    screen_center_y = screen_height / 2;

    x_error_occurred = 0;
    XSelectInput(display, root,
                 SubstructureRedirectMask | SubstructureNotifyMask |
                 PropertyChangeMask | KeyPressMask | ButtonPressMask |
                 ButtonReleaseMask | PointerMotionMask);
    sync_display();
    if (x_error_occurred != 0) {
        fprintf(stderr, "[Fatal] Failed to select input on root. Another WM may be running.
");
        XCloseDisplay(display);
        return 1;
    }

    query_and_manage_existing_windows();
    register_global_hotkeys();
    set_cursor(root, XC_arrow);

    const long frame_ns = 16666667L;
    struct timespec last_frame;
    clock_gettime(CLOCK_MONOTONIC, &last_frame);

    while (1) {
        process_pending_events();
        update_orbit_positions();

        struct timespec current_frame;
        clock_gettime(CLOCK_MONOTONIC, &current_frame);
        long elapsed_ns = (current_frame.tv_sec - last_frame.tv_sec) * 1000000000L +
                          (current_frame.tv_nsec - last_frame.tv_nsec);
        if (elapsed_ns < frame_ns) {
            struct timespec sleep_time = {0, frame_ns - elapsed_ns};
            nanosleep(&sleep_time, NULL);
        }
        clock_gettime(CLOCK_MONOTONIC, &last_frame);
    }

    XCloseDisplay(display);
    return 0;
}
