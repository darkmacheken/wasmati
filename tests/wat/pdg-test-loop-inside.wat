(module
  (func $add (result i32) (local $x i32) (local $y i32) (local $z i32) (local $w i32)
    local.get $x
    local.set $y
    loop $loop
      block $block
        local.get $z
        if 
          loop $loop2
            block $block2
              local.get $z
              if 
                br $block2
              else
                local.get $y
                local.set $z
                br $loop2
              end
            end
            nop
          end
          br $block
        else
          local.get $y
          local.set $z
          br $loop
        end
      end
      nop
    end
    local.get $z
  )
)
