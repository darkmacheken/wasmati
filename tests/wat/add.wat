(module
  (global $threwValue (mut i32) (i32.const 0))
  (global $setjmpId (mut i32) (i32.const 0))
  ;; Export this function under the name "add".
  ;; Note: exports have only a single "level", unlike imports, which have two.
  (func (export "add") (param i32 i32) (result i32)
    ;; Function arguments are (implicitly) in the first n_arg locals.
    ;; Locals are function-wide variables that are independent of the stack.
    ;; local.get "loads" a local onto the stack.
    local.get 1
    if (result i32)
      block (result i32) 
        local.get 0
        i32.const 1
        i32.add
      end
    else
      block (result i32)
        local.get 0
        i32.const 2
        i32.add
      end
      if (result i32)
        local.get 0
        unreachable
        i32.const 4
        i32.add
      else
        local.get 1
        i32.const 5
        i32.add
      end
    end
    i32.const 3
    i32.add
    ;; Implicitly return the top value on the stack as the function result.
  )
)
