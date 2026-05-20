mod wm;
mod config;
mod events;
mod layout;
mod keys;
mod util;

use anyhow::Result;
use wm::WindowManager;
use config::load_config;

fn main() -> Result<()> {
    let config = load_config();
    let mut wm = WindowManager::new(config)?;
    wm.init()?;
    println!("ARWM is now managing windows.");
    wm.run()?;
    Ok(())
}
