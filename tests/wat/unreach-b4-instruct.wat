;;; TOOL: wat2wasm
(module
  (func
      i32.const 4
      i32.const 2
      unreachable
      i32.add
      drop
  ))
