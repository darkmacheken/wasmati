;;; TOOL: wat2wasm
(module
  (func $main (param $x i32) (param $y i32) (result i32)
    block $exit (result i32)
      i32.const 2
      local.get $x
      br_if 0
      local.get $y
      i32.add
    end)
  (export "main" (func $main))
)