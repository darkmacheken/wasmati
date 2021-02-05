;;; TOOL: wat2wasm
(module
  (type $t (func (result i32)))
  (table 2 anyfunc)
  (elem (i32.const 0) $test)
  (elem (i32.const 1) $test2)
  (memory $0 1)

  (func $test (result i32)
    (i32.const 42)
  )

    (func $test2 (result i32)
    (i32.const 42)
  )

  (func $main (result i32)
    (call_indirect (type $t)
      (i32.const 0)
    )
  )

)
