;;; TOOL: wat2wasm
(module
  (func (param $x i32) (param $y i32) (result i32)
    block $test (result i32)
      local.get $x
      local.get $y
      br $test
      if $exit 
        br $exit
      end
    end
  ))
