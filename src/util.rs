use std::process::Command;

pub fn spawn(cmd: &str) {
    let _ = Command::new("sh")
        .arg("-c")
        .arg(cmd)
        .spawn();
}
