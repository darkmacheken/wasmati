WITH ["$malloc","$dlmalloc"] as mallocs, ["$free","$dlfree"] as frees

MATCH (f:Function)-[:AST*1..]->(i:Instruction)
WHERE i.instType="Call" AND i.label IN mallocs

WITH *
MATCH (i)-[:CFG*..]->(free:Instruction)
WHERE free.instType="Call" AND free.label IN frees AND
	(i)-[:PDG*1.. {pdgType:"Function", label:i.label}]->(free)
    
WITH *
MATCH (free)-[:CFG*1..]->(uaf:Instruction)
WHERE uaf.instType="Call" AND uaf.label IN frees AND
	(i)-[:PDG*1.. {pdgType:"Function", label:i.label}]->(uaf)    

RETURN uaf.label as caller, f.name as function;