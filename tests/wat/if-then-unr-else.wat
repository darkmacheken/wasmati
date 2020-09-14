;;; TOOL: wat2wasm
(module
  (func
    i32.const 1
    if 
      nop
      unreachable
    else
      nop
    end
    nop))
