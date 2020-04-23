(module
  (memory 1)
  (func (param $x i32)
    local.get $x
    memory.grow
    drop
    memory.size
    drop))