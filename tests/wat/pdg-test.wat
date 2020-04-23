(module
  (func $add (result i32) (local $x i32) (local $y i32) (local $z i32)
    local.get $x 
    if (result i32) 
      local.get $y
      i32.const 2
      i32.add
    else
      local.get $z
      i32.const 3
      i32.mul
    end
    i32.const 1
    i32.add
  )
)
