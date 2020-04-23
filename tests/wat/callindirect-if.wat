;;; TOOL: wat2wasm
(module
  (table anyfunc (elem 0))
  (type (func (param f32)))
  (func
    i32.const 0
    if (result f32)
      f32.const 0
    else
      f32.const 1
    end
    i32.const 0
    call_indirect (type 0)))
