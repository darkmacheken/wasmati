// List of vulnerable functions
WITH apoc.map.fromPairs([["$read", [1, 2]], ["$fgets", [0, 1]]]) AS boFuncs, ["$dlmalloc", "$malloc] as mallocs

MATCH (f:Function)-[:AST*1..]->(m:Instruction)<-[mSizeEdge:PDG {pdgType:"Const"}]-()
WHERE m.instType="Call"
	AND m.label IN mallocs
    
WITH *
MATCH (f)-[:AST*1..]->(r:Instruction)-[argEdge:AST]->(rSize:Instruction)
WHERE r.instType="Call"
	AND r.label IN keys(boFuncs)
    AND argEdge.arg=boFuncs[r.label][1]
    AND (m)-[:PDG*1.. {pdgType:"Function", label:m.label}]->(r)
    
WITH * 
MATCH (rSize)-[rSizeEdge:PDG {pdgType:"Const"}]->(r)
WHERE rSizeEdge.value_i >= mSizeEdge.value_i

RETURN  f.name AS function, mSizeEdge.value_i AS expected_size, rSizeEdge.value_i AS read_size;