// Get loops
MATCH (f:Function)-[:AST*1..]->(loop:Instruction)
WHERE loop.instType="Loop"

// Get stores and vars
WITH *
MATCH (loop)-[:AST*1..]->(store:Instruction)-[argEdge:AST]->(storeArg:Instruction)-[:AST]->(var:Instruction)
WHERE store.instType="Store"
	AND argEdge.arg=0
    AND storeArg.instType="Binary"
    AND storeArg.opcode="i32.add"
    AND (
    	var.instType="LocalGet" OR
       	var.instType="LocalTee"
    )

// Get add instructions in function
WITH *
MATCH (f)-[:AST*1..]->(add:Instruction)
WHERE add.instType="Binary"
	AND add.opcode="i32.add"
    
// Check if var is being incremented with a constant
WITH *
MATCH (childConst:Instruction)<-[:AST]-(add)-[:AST]->(childLocal:Instruction)
WHERE childConst.instType="Const"
	AND childLocal.label=var.label
    AND (
    	childLocal.instType="LocalGet" OR
       	childLocal.instType="LocalTee"
    )

// Check if breaks depend on store vars
WITH *
OPTIONAL MATCH path=(loop)-[:AST*1..]->(brIf:Instruction)-[:AST]->(compare:Instruction)-[:AST*1..]->(local:Instruction)
WHERE brIf.instType="BrIf"
	AND compare.instType="Compare"
    AND local.label=var.label
    AND (
    	var.instType="LocalGet" OR
       	var.instType="LocalTee"
    )

WITH path, f, loop
RETURN DISTINCT
CASE path WHEN path=null THEN f.name END AS function,
CASE path WHEN path=null THEN loop.label END AS loop;