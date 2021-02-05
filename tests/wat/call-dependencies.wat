;;; TOOL: wat2wasm
(module
  (func $foo (param $x i32) (param $y i32)
    local.get $x
    local.get $y
    call $bar
    drop)
  (func $bar (param i32 i32) (result i32)
    i32.const 0))
