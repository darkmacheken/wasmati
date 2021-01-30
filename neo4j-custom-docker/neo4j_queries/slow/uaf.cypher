WITH ["$dlmalloc"] as mallocs, ["$dlfree"] as frees
MATCH (f:Function),(i:Instruction),(free:Instruction),(uaf:Instruction)
WHERE i.instType="Call" AND i.label IN mallocs AND
    free.instType="Call" AND free.label IN frees AND
	(f)-[:AST*1..]->(i)-[:CFG*..]->(free)-[:CFG*1..]->(uaf) AND
	(i)-[:PDG*1.. {pdgType:"Function", label:i.label}]->(free) AND
	(i)-[:PDG*1.. {pdgType:"Function", label:i.label}]->(uaf)    
RETURN uaf.label as caller, f.name as function;