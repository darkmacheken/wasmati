;;; TOOL: wat2wasm
(module
  (type $t (func (result i32)))
  (table 1 anyfunc)
  (elem (i32.const 0) $test)
  (memory $0 1)

  (func $test (type $t) (result i32)
    (i32.const 42)
  )

  (func $main (result i32)
    (call_indirect (type $t)
      (i32.const 0)
    )
  )

)
