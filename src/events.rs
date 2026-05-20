use xcb::x;
use crate::wm::WindowManager;
use anyhow::Result;

pub fn handle_event(wm: &mut WindowManager, event: xcb::Event) -> Result<bool> {
    match event {
        xcb::Event::X(x::Event::MapRequest(ev)) => {
            wm.manage(ev.window())?;
        }
        xcb::Event::X(x::Event::UnmapNotify(ev)) => {
            wm.unmanage(ev.window())?;
        }
        xcb::Event::X(x::Event::DestroyNotify(ev)) => {
            wm.unmanage(ev.window())?;
        }
        xcb::Event::X(x::Event::ConfigureRequest(ev)) => {
            wm.handle_configure_request(ev)?;
        }
        xcb::Event::X(x::Event::KeyPress(ev)) => {
            if !wm.handle_key_press(ev)? {
                return Ok(false);
            }
        }
        xcb::Event::X(x::Event::EnterNotify(ev)) => {
            wm.focus(ev.event())?;
        }
        _ => {}
    }
    Ok(true)
}
