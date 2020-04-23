(module
  (func $main (param $x i32) (param $y i32) (result i32)
    block $B0
      block $B1
        block $B2
          block $B3
            block $B4
              block $B5
                local.get $x
                br_table $B5 $B4 $B3 $B2 $B1 
              end
              i32.const 1
              local.set $y
              br $B0
            end
            i32.const 2
            local.set $y
            br $B0
          end
          i32.const 3
          local.set $y
          br $B0
        end
        i32.const 4
        local.set $y
        br $B0
      end
    end
    local.get $y
  )
)