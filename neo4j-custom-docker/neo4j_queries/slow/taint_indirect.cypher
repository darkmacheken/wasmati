WITH ["$read_bytes_to_mmap_memory"] as sources
MATCH (f:Function),(call_indirect:Instruction),(source_call:Instruction)
WHERE call_indirect.instType="CallIndirect" AND source_call.instType="Call" AND source_call.label IN sources AND
    (f)-[:AST*1..]->(source_call) AND
    (source_call)-[:PDG*1.. {pdgType:"Function", label:source_call.label}]->(call_indirect) AND
    (call_indirect)-[:AST {arg:call_indirect.nargs-1}]->(source_call)
RETURN source_call.label as source, call_indirect.label as sink, f.name as function;