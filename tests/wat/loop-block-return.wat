;;; TOOL: wat2wasm
(module
  (func (result i32)
    loop $L0 (result i32) 
      block
        i32.const 1
        return
      end
      br $L0
    end))
