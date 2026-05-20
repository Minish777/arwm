use xcb::x;
use anyhow::{Result, anyhow};
use crate::config::Config;
use crate::layout::calculate_tiling;
use crate::util::spawn;
use crate::keys::Action;

pub struct WindowManager {
    conn: xcb::Connection,
    screen_num: i32,
    config: Config,
    clients: Vec<x::Window>,
    focused: Option<x::Window>,
    floating_clients: Vec<x::Window>,
    fullscreen_client: Option<x::Window>,

    // Keycodes (Default mappings)
    key_enter: x::Keycode,
    key_d: x::Keycode,
    key_q: x::Keycode,
    key_f: x::Keycode,
    key_space: x::Keycode,
    key_h: x::Keycode,
    key_j: x::Keycode,
    key_k: x::Keycode,
    key_l: x::Keycode,
}

impl WindowManager {
    pub fn new(config: Config) -> Result<Self> {
        let (conn, screen_num) = xcb::Connection::connect(None)?;

        Ok(Self {
            conn,
            screen_num,
            config,
            clients: vec![],
            focused: None,
            floating_clients: vec![],
            fullscreen_client: None,
            key_enter: 36,
            key_d: 40,
            key_q: 24,
            key_f: 41,
            key_space: 65,
            key_h: 43,
            key_j: 44,
            key_k: 45,
            key_l: 46,
        })
    }

    pub fn init(&mut self) -> Result<()> {
        let setup = self.conn.get_setup();
        let screen = setup.roots().nth(self.screen_num as usize).ok_or_else(|| anyhow!("No screen found"))?;

        let mask = x::Cw::EventMask(
            x::EventMask::SUBSTRUCTURE_REDIRECT |
            x::EventMask::SUBSTRUCTURE_NOTIFY |
            x::EventMask::STRUCTURE_NOTIFY |
            x::EventMask::PROPERTY_CHANGE
        );

        let cookie = self.conn.send_request_checked(&x::ChangeWindowAttributes {
            window: screen.root(),
            value_list: &[mask],
        });

        self.conn.check_request(cookie).map_err(|_| anyhow!("Another window manager is running"))?;

        self.grab_keys(screen.root())?;

        // Initial terminal spawn to avoid black screen
        spawn(&self.config.terminal);

        // Autostart
        let config_dir = crate::config::get_config_dir()?;
        let autostart_path = config_dir.join("autostart.sh");
        if autostart_path.exists() {
            spawn(&format!("sh {}", autostart_path.display()));
        }

        self.conn.flush()?;
        Ok(())
    }

    fn grab_keys(&self, root: x::Window) -> Result<()> {
        let modifiers = x::ModMask::N4;
        let keys = [
            self.key_enter, self.key_d, self.key_q, self.key_f,
            self.key_space, self.key_h, self.key_j, self.key_k, self.key_l
        ];

        for key in keys {
            self.conn.send_request(&x::GrabKey {
                owner_events: true,
                grab_window: root,
                modifiers,
                key,
                pointer_mode: x::GrabMode::Async,
                keyboard_mode: x::GrabMode::Async,
            });

            if key == self.key_q {
                self.conn.send_request(&x::GrabKey {
                    owner_events: true,
                    grab_window: root,
                    modifiers: modifiers | x::ModMask::SHIFT,
                    key,
                    pointer_mode: x::GrabMode::Async,
                    keyboard_mode: x::GrabMode::Async,
                });
            }
        }
        Ok(())
    }

    pub fn manage(&mut self, win: x::Window) -> Result<()> {
        if self.clients.contains(&win) { return Ok(()); }

        let attr_cookie = self.conn.send_request(&x::GetWindowAttributes { window: win });
        let attr = self.conn.wait_for_reply(attr_cookie)?;
        if attr.override_redirect() {
            self.conn.send_request(&x::MapWindow { window: win });
            return Ok(());
        }

        self.clients.push(win);
        let mask = x::Cw::EventMask(x::EventMask::ENTER_WINDOW | x::EventMask::FOCUS_CHANGE);
        self.conn.send_request(&x::ChangeWindowAttributes {
            window: win,
            value_list: &[mask],
        });

        self.conn.send_request(&x::MapWindow { window: win });
        self.refresh()?;
        self.focus(win)?;
        Ok(())
    }

    pub fn unmanage(&mut self, win: x::Window) -> Result<()> {
        self.clients.retain(|&c| c != win);
        self.floating_clients.retain(|&c| c != win);
        if self.fullscreen_client == Some(win) {
            self.fullscreen_client = None;
        }
        if self.focused == Some(win) {
            self.focused = self.clients.last().copied();
        }
        self.refresh()?;
        Ok(())
    }

    pub fn focus(&mut self, win: x::Window) -> Result<()> {
        if !self.clients.contains(&win) { return Ok(()); }

        self.conn.send_request(&x::SetInputFocus {
            revert_to: x::InputFocus::PointerRoot,
            focus: win,
            time: x::CURRENT_TIME,
        });

        self.focused = Some(win);
        self.update_borders()?;
        self.conn.flush()?;
        Ok(())
    }

    fn update_borders(&self) -> Result<()> {
        for &win in &self.clients {
            let color = if Some(win) == self.focused { self.config.border_color_focus } else { self.config.border_color_normal };
            self.conn.send_request(&x::ChangeWindowAttributes {
                window: win,
                value_list: &[x::Cw::BorderPixel(color)],
            });
            self.conn.send_request(&x::ConfigureWindow {
                window: win,
                value_list: &[x::ConfigWindow::BorderWidth(self.config.border_width)],
            });
        }
        Ok(())
    }

    pub fn handle_configure_request(&self, ev: x::ConfigureRequestEvent) -> Result<()> {
        let mut vals = vec![];
        if ev.value_mask().contains(x::ConfigWindowMask::X) { vals.push(x::ConfigWindow::X(ev.x() as i32)); }
        if ev.value_mask().contains(x::ConfigWindowMask::Y) { vals.push(x::ConfigWindow::Y(ev.y() as i32)); }
        if ev.value_mask().contains(x::ConfigWindowMask::WIDTH) { vals.push(x::ConfigWindow::Width(ev.width() as u32)); }
        if ev.value_mask().contains(x::ConfigWindowMask::HEIGHT) { vals.push(x::ConfigWindow::Height(ev.height() as u32)); }

        self.conn.send_request(&x::ConfigureWindow {
            window: ev.window(),
            value_list: &vals,
        });
        Ok(())
    }

    pub fn handle_key_press(&mut self, ev: x::KeyPressEvent) -> Result<bool> {
        let code = ev.detail();
        let state = ev.state();

        let action = if code == self.key_enter {
            Some(Action::Terminal)
        } else if code == self.key_d {
            Some(Action::Launcher)
        } else if code == self.key_q {
            if state.contains(x::KeyButMask::SHIFT) {
                Some(Action::Quit)
            } else {
                Some(Action::Close)
            }
        } else if code == self.key_f {
            Some(Action::Fullscreen)
        } else if code == self.key_space {
            Some(Action::ToggleFloating)
        } else if [self.key_h, self.key_k].contains(&code) {
            Some(Action::FocusPrev)
        } else if [self.key_j, self.key_l].contains(&code) {
            Some(Action::FocusNext)
        } else {
            None
        };

        if let Some(act) = action {
            match act {
                Action::Terminal => spawn(&self.config.terminal),
                Action::Launcher => spawn(&self.config.launcher),
                Action::Close => if let Some(win) = self.focused {
                    self.conn.send_request(&x::DestroyWindow { window: win });
                },
                Action::Quit => return Ok(false),
                Action::Fullscreen => if let Some(win) = self.focused {
                    if self.fullscreen_client == Some(win) {
                        self.fullscreen_client = None;
                    } else {
                        self.fullscreen_client = Some(win);
                    }
                    self.refresh()?;
                },
                Action::ToggleFloating => if let Some(win) = self.focused {
                    if self.floating_clients.contains(&win) {
                        self.floating_clients.retain(|&c| c != win);
                    } else {
                        self.floating_clients.push(win);
                    }
                    self.refresh()?;
                },
                Action::FocusNext | Action::FocusPrev => if !self.clients.is_empty() {
                    let idx = self.focused.and_then(|f| self.clients.iter().position(|&c| c == f)).unwrap_or(0);
                    let next_idx = if act == Action::FocusNext {
                        (idx + 1) % self.clients.len()
                    } else {
                        (idx + self.clients.len() - 1) % self.clients.len()
                    };
                    self.focus(self.clients[next_idx])?;
                }
            }
        }

        Ok(true)
    }

    pub fn refresh(&self) -> Result<()> {
        let setup = self.conn.get_setup();
        let screen = setup.roots().nth(self.screen_num as usize).unwrap();
        let sw = screen.width_in_pixels() as u32;
        let sh = screen.height_in_pixels() as u32;

        if let Some(win) = self.fullscreen_client {
            self.conn.send_request(&x::ConfigureWindow {
                window: win,
                value_list: &[
                    x::ConfigWindow::X(0),
                    x::ConfigWindow::Y(0),
                    x::ConfigWindow::Width(sw),
                    x::ConfigWindow::Height(sh),
                    x::ConfigWindow::BorderWidth(0),
                ],
            });
            return Ok(());
        }

        let tiled_clients: Vec<x::Window> = self.clients.iter()
            .filter(|c| !self.floating_clients.contains(c))
            .copied()
            .collect();

        let rects = calculate_tiling(sw, sh, tiled_clients.len(), self.config.gap);

        for (i, &win) in tiled_clients.iter().enumerate() {
            let r = rects[i];
            self.conn.send_request(&x::ConfigureWindow {
                window: win,
                value_list: &[
                    x::ConfigWindow::X(r.x),
                    x::ConfigWindow::Y(r.y),
                    x::ConfigWindow::Width(r.w),
                    x::ConfigWindow::Height(r.h),
                    x::ConfigWindow::BorderWidth(self.config.border_width),
                ],
            });
        }
        self.conn.flush()?;
        Ok(())
    }

    pub fn run(&mut self) -> Result<()> {
        loop {
            let event = self.conn.wait_for_event()?;
            if !crate::events::handle_event(self, event)? {
                break;
            }
        }
        Ok(())
    }
}
