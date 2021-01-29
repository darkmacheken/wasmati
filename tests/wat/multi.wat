;;; TOOL: wat2wasm
(module
  (func
    block $test (result i32 i32)
      i32.const 4
      i32.const 2
    end
    i32.add
    drop
    ))
