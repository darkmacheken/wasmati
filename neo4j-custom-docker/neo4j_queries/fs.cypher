WITH apoc.map.fromPairs([
    ["$fprintf",1],
    ["$printf",0],
    ["$iprintf",0],
    ["$sprintf",1],
    ["$snprintf",2],
    ["$vfprintf",1],
    ["$vprintf",0],
    ["$vsprintf",1],
    ["$vsnprintf",2],
    ["$syslog",1],
    ["$vsyslog",1]]) as formatStrings

MATCH (f:Function)-[:AST*1..]->(i:Instruction)-[r:AST]->(child)
WHERE NOT (child)-[:PDG {pdgType:"Const"}]->(i) AND
    i.instType="Call" AND i.label IN keys(formatStrings) AND
    r.arg=formatStrings[i.label]
RETURN i.label as caller,f.name as function;