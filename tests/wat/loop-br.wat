;;; TOOL: wat2wasm
(module
  (func
    loop $loop
      block $block
        i32.const 0
        if 
          br $block
        else
          br $loop
        end
      end
      nop
    end))
