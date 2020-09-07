;;; TOOL: wat2wasm
(module
  (func
    block $test 
      i32.const 4
      i32.const 2
      br $test
      if $exit 
        br $exit
      end
    end
  ))
