;;; TOOL: wat2wasm
(module
  (func
    i32.const 0
    loop $loop
      block $block
        if 
          br $block
        else
          br $loop
        end
      end
      nop
    end))
