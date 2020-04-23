;;; TOOL: wat2wasm
(module
  (func $main (result i32)
    block $exit (result i32)
      i32.const 0
      i32.const 1
      br_if 0
      drop  
      i32.const 1
    end)
  (export "main" (func $main))
)