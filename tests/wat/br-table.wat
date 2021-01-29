(module
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
              (br 4 (;@2;)))
            (i32.store
              (i32.const 0)
              (i32.const 2))
            (br 3 (;@3;)))
          (i32.store
            (i32.const 0)
            (i32.const 3))
          (br 2 (;@4;)))
        (i32.store
          (i32.const 0)
          (i32.const 4))
        (br 1 (;@5;))))
  )
  (export "main" (func $main))
  (memory (;0;) 2)
)