;;; TOOL: wat2wasm
(module
  (table anyfunc (elem 0))
  (type (func (param f32 i32) ))
  (func (param $x i32) (param $y f32)
    local.get $y ;; arg
    i32.const 2  ;; arg2
    local.get $x ;; index
    call_indirect (type 0)))
