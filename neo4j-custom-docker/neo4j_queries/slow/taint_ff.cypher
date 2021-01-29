WITH ["$sink"] as sinks, ["$source"] as sources
MATCH (f:Function),(sink_call:Instruction),(source_call:Instruction)
WHERE sink_call.instType="Call" AND sink_call.label IN sinks AND
	source_call.instType="Call" AND source_call.label IN sources AND
    (f)-[:AST*1..]->(sink_call) AND
    (source_call)-[:PDG*1.. {pdgType:"Function", label:source_call.label}]->(sink_call)
RETURN source_call.label as source, sink_call.label as sink, f.name as function;