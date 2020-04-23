;;; TOOL: wat2wasm
(module
  (func 
    block
      block
        nop
        br 0
        i32.const 1
        drop
      end
      nop
      nop
    end
  )
)