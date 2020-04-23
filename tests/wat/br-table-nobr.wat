(module
  (type (;0;) (func (result i32)))
  (type (;1;) (func))
  (type (;2;) (func (result i32)))
  (type (;3;) (func (param i32)))
  (import "env" "random" (func $random (type 0)))
  (func $__wasm_call_ctors (type 1))
  (func $main
    (block  ;; label = @1
      (block  ;; label = @2
        (block  ;; label = @3
          (block  ;; label = @4
            (block  ;; label = @5
              (block  ;; label = @6
                (br_table 0 (;@6;) 1 (;@5;) 2 (;@4;) 3 (;@3;) 4 (;@2;)
                  (i32.const 0)))
              (i32.store
                (i32.const 0)
                (i32.const 1))
             )
            (i32.store
              (i32.const 0)
              (i32.const 2))
            )
          (i32.store
            (i32.const 0)
            (i32.const 3))
          )
        (i32.store
          (i32.const 0)
          (i32.const 4))
        ))
  )
  (table (;0;) 1 1 funcref)
  (memory (;0;) 2)
  (global (;0;) (mut i32) (i32.const 66816))
  (global (;1;) i32 (i32.const 66816))
  (global (;2;) i32 (i32.const 1280))
  (export "main" (func $main))
  (export "memory" (memory 0))
  (export "__heap_base" (global 1))
  (export "__data_end" (global 2))
  (data (;0;) (i32.const 1024) "\14\00\00\00\d6\00\00\00\9a\03\00\00\0b\02\00\00\f4\00\00\00}\00\00\00\ae\11\00\00\cb\04\00\00")
  (data (;1;) (i32.const 1056) "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00")
  (data (;2;) (i32.const 1136) "\1f\00\00\00\84\04\00\00\03\00\00\00\00\00\00\00\00\00\00\00-\f4QX\cf\8c\b1\c0F\f6\b5\cb)1\03\c7\04[p0\b4]\fd x\7f\8b\9a\d8Y)PhH\89\ab\a7V\03l\ff\b7\cd\88?\d4w\b4+\a5\a3p\f1\ba\e4\a8\fcA\83\fd\d9o\e1\8az/-t\96\07\1f\0d\09^\03v,p\f7@\a5,\a7oWA\a8\aat\df\a0Xd\03J\c7\c4<S\ae\af_\18\04\15\b1\e3m(\86\ab\0c\a4\bfC\f0\e9P\819W\16R7"))
