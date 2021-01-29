;;; TOOL: wat2wasm
(module
  (func
    loop $L1
      nop
      nop
      i32.const 0
      if
        nop
        i32.const 0
        br_if $L1
      end
    end
    nop))
