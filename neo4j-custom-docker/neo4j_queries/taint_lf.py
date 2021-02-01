from neo4j import GraphDatabase

driver = GraphDatabase.driver("bolt://localhost:7687/")
session = driver.session()

def searchForTaintedCalls():
    # Initial Tainted Functions
    tainted = [["$main", [0, 1]]]
    sinks = ["$strcpy"]
    taintedCalls = []
    visited = set()

    while len(tainted) > 0:
        # query = """
        # WITH apoc.map.fromPairs({0}) AS taintedInitial

        # MATCH (taintedF:Function)-[:AST*..]->(:Parameters)-[:AST]->(p)
        # WHERE taintedF.name IN keys(taintedInitial) AND p.index IN taintedInitial[taintedF.name]

        # WITH *
        # MATCH (taintedF)-[:AST*1..]->(call:Instruction)-[callArgIndex:AST]->()-[:AST*0..]->(callArg)
        # WHERE call.instType="Call" AND callArg.label=p.name

        # RETURN taintedF.name AS taintedFunction, p.index AS taintedParam, call.label AS callInst, collect(DISTINCT callArgIndex.arg) AS callInstParams
        # """.format(tainted)

        query_pdg="""
        WITH apoc.map.fromPairs({0}) AS taintedInitial

        MATCH (taintedF:Function)-[:AST*..]->(:Parameters)-[:AST]->(p)
        WHERE taintedF.name IN keys(taintedInitial) AND p.index IN taintedInitial[taintedF.name]

        WITH *
        MATCH (taintedF)-[:AST*1..]->(call:Instruction)-[:AST*0..1]->(s)<-[:PDG*1.. {{pdgType:"Local",label:p.name}}]-(arg:Instruction)
        WHERE call.instType="Call"

        WITH *
        MATCH (call)-[callArgIndex:AST]->()-[:AST*0..]->(arg)

        RETURN DISTINCT taintedF.name AS taintedFunction, p.index AS taintedParam, call.label AS callInst, collect(DISTINCT callArgIndex.arg) AS callInstParams
        """.format(tainted)

        queryExecution = session.run(query_pdg)
        tainted = []
        for (taintedFunction, taintedParam, callInst, callInstParams) in queryExecution:
            visited.add(taintedFunction)
            if callInst in sinks:
                taintedCalls.append((taintedFunction, taintedParam, callInst, callInstParams))
            elif not callInst in visited:
                tainted.append([callInst, callInstParams])

    print("taintedFunction,taintedParam,sink")
    for (taintedFunction, taintedParam, callInst, callInstParams) in taintedCalls:
        print("{0},{1},{2}".format(taintedFunction, taintedParam, callInst))

searchForTaintedCalls()