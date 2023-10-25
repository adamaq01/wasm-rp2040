use wasm_bindgen::prelude::*;

#[wasm_bindgen(module = "GPIO")]
extern "C" {
    fn gpio_init(gpio: i32);
    fn gpio_set_dir(gpio: i32, out: u8);
    fn gpio_put(gpio: i32, value: u8);
}

#[wasm_bindgen(module = "Time")]
extern "C" {
    #[wasm_bindgen(js_name = get)]
    fn time_get() -> u64;
}

#[wasm_bindgen]
pub fn main() {
    let gpio = 25;
    gpio_init(gpio);
    gpio_set_dir(gpio, 1);
    loop {
        gpio_put(gpio, 1);
        let start = time_get();
        while time_get() - start < 500000 {}
        gpio_put(gpio, 0);
        let start = time_get();
        while time_get() - start < 500000 {}
    }
}
