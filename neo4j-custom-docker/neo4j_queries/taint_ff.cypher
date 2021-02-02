WITH ["$sink"] as sinks, ["$source", "$read_bytes_to_mmap_memory"] as sources

MATCH (f:Function)-[:AST*1..]->(sink_call:Instruction),(source_call:Instruction)
WHERE sink_call.instType="Call"
	AND sink_call.label IN sinks

WITH *
MATCH (source_call:Instruction)-[:PDG*1..]->(sink_call)
WHERE source_call.instType="Call"
	AND source_call.label IN sources
    AND (source_call)-[:PDG*1.. {pdgType:"Function", label:source_call.label}]->(sink_call)

RETURN source_call.label as source, sink_call.label as sink, f.name as function;