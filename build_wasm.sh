#!/bin/bash

cd hello
cargo build --target wasm32-unknown-unknown --release
wasm-bindgen target/wasm32-unknown-unknown/release/hello.wasm --out-dir . --no-typescript --remove-producers-section --omit-default-module-path
rm hello_bg.js hello.js
wasm-opt -Os -o hello.wasm hello_bg.wasm
rm hello_bg.wasm
xxd -iC hello.wasm > ../src/hello.wasm.h
rm hello.wasm