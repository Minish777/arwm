use serde::{Deserialize, Serialize};
use std::fs;
use std::path::PathBuf;
use directories::ProjectDirs;
use anyhow::Result;

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct Config {
    pub terminal: String,
    pub launcher: String,
    pub gap: u32,
    pub border_width: u32,
    pub border_color_focus: u32,
    pub border_color_normal: u32,
}

impl Default for Config {
    fn default() -> Self {
        Self {
            terminal: "alacritty".to_string(),
            launcher: "dmenu_run".to_string(),
            gap: 10,
            border_width: 2,
            border_color_focus: 0x61AFEF,
            border_color_normal: 0x3E4452,
        }
    }
}

pub fn get_config_dir() -> Result<PathBuf> {
    let dirs = ProjectDirs::from("", "", "arwm").ok_or_else(|| anyhow::anyhow!("Could not find config directory"))?;
    let path = dirs.config_dir().to_path_buf();
    if !path.exists() {
        fs::create_dir_all(&path)?;
    }
    Ok(path)
}

pub fn load_config() -> Config {
    let path = get_config_dir().map(|p| p.join("arwm.toml")).unwrap_or_default();
    if !path.exists() {
        let default = Config::default();
        let _ = fs::write(&path, toml::to_string(&default).unwrap_or_default());
        return default;
    }

    let content = fs::read_to_string(path).unwrap_or_default();
    toml::from_str(&content).unwrap_or_default()
}
