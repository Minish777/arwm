#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Action {
    Terminal,
    Launcher,
    Close,
    Quit,
    Fullscreen,
    ToggleFloating,
    FocusNext,
    FocusPrev,
}
