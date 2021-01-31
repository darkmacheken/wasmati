from neo4j import GraphDatabase

driver = GraphDatabase.driver("bolt://localhost:7687/")
session = driver.session()

def searchForTaintedCalls():
    # Initial Tainted Functions
    tainted = [["$main", [0, 1]]]
    sinks = ["$strcpy"]
    taintedCalls = []

    while len(tainted) > 0:
        query = """
        WITH apoc.map.fromPairs({0}) AS taintedInitial

        MATCH path1=(taintedF:Function)-[:AST*..]->(:Parameters)-[:AST]->(p)
        WHERE taintedF.name IN keys(taintedInitial) AND p.index IN taintedInitial[taintedF.name]

        WITH *
        MATCH path2=(taintedF)-[:AST*1..]->(call:Instruction)-[callArgIndex:AST]->()-[:AST*0..]->(callArg)
        WHERE call.instType="Call" AND callArg.label=p.name

        RETURN taintedF.name AS taintedFunction, p.index AS taintedParam, call.label AS callInst, collect(DISTINCT callArgIndex.arg) AS callInstParams
        """.format(tainted)

        queryExecution = session.run(query)
        tainted = []
        for (taintedFunction, taintedParam, callInst, callInstParams) in queryExecution:
            if callInst in sinks:
                taintedCalls.append((taintedFunction, taintedParam, callInst, callInstParams))
            else:
                tainted.append([callInst, callInstParams])

    print("taintedFunction,taintedParam,sink")
    for (taintedFunction, taintedParam, callInst, callInstParams) in taintedCalls:
        print("{0},{1},{2}".format(taintedFunction, taintedParam, callInst))

searchForTaintedCalls()