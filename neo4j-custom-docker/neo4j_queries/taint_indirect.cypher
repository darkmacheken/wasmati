WITH ["$read_bytes_to_mmap_memory", "$source"] as sources

MATCH (f:Function)-[:AST*1..]->(source_call:Instruction)
WHERE source_call.instType="Call"
	AND source_call.label IN sources
    
WITH *
MATCH (call_indirect:Instruction)-[astEdge:AST]->(source_call)
WHERE call_indirect.instType="CallIndirect"
	AND astEdge.arg=call_indirect.nargs-1
    AND (source_call)-[:PDG*1.. {pdgType:"Function", label:source_call.label}]->(call_indirect)

RETURN source_call.label as source, call_indirect.label as sink, f.name as function;