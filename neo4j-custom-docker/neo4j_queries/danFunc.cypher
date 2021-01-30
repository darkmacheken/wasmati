WITH ["$gets"] as DANGEROUS_FUNC
MATCH (f:Function)-[:AST*1..]->(i:Instruction)
WHERE i.instType="Call" AND i.label IN DANGEROUS_FUNC
RETURN i.label as caller,f.name as function;