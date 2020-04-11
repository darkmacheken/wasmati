;;; TOOL: wat2wasm
(module
  (func 
    block
      i32.const 1
      i32.const 0
      br_if 0
      if 
        nop
      else
        nop
      end
    end
  )
)