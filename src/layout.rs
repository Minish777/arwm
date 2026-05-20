#[derive(Debug, Clone, Copy)]
pub struct Rect {
    pub x: i32,
    pub y: i32,
    pub w: u32,
    pub h: u32,
}

pub fn calculate_tiling(screen_w: u32, screen_h: u32, count: usize, gap: u32) -> Vec<Rect> {
    if count == 0 { return vec![]; }
    if count == 1 {
        return vec![Rect {
            x: gap as i32,
            y: gap as i32,
            w: screen_w - (gap * 2),
            h: screen_h - (gap * 2),
        }];
    }

    let mut rects = Vec::with_capacity(count);
    let master_w = screen_w / 2;

    // Master
    rects.push(Rect {
        x: gap as i32,
        y: gap as i32,
        w: master_w - (gap * 2),
        h: screen_h - (gap * 2),
    });

    // Stack
    let stack_h = screen_h / (count - 1) as u32;
    for i in 0..(count - 1) {
        rects.push(Rect {
            x: (master_w + gap) as i32,
            y: (i as u32 * stack_h + gap) as i32,
            w: (screen_w - master_w) - (gap * 2),
            h: stack_h - (gap * 2),
        });
    }

    rects
}
