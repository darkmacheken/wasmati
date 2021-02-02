from neo4j import GraphDatabase
import json
import os

driver = GraphDatabase.driver("bolt://localhost:7687/")
session = driver.session()

def searchForMallocBufferOverflows():
    configFile = os.path.join(os.path.dirname(__file__), "defaultConfigs.json")
    with open(configFile, "r") as configs:
        configurations = json.load(configs)
        bufferOverflowFunctions = configurations["bufferOverflow"]
        boFuncs = []
        for boFunc, parameters in iter(bufferOverflowFunctions.items()):
            boFuncs.append([boFunc, [parameters["buffer"], parameters["size"]]])

        mallocs = configurations["malloc"]

        query="""
        // List of vulnerable functions
        WITH apoc.map.fromPairs({0}) AS boFuncs, {1} as mallocs

        MATCH (f:Function)-[:AST*1..]->(m:Instruction)<-[mSizeEdge:PDG {{pdgType:"Const"}}]-()
        WHERE m.instType="Call"
            AND m.label IN mallocs
            
        WITH *
        MATCH (f)-[:AST*1..]->(r:Instruction)-[argEdge:AST]->(rSize:Instruction)
        WHERE r.instType="Call"
            AND r.label IN keys(boFuncs)
            AND argEdge.arg=boFuncs[r.label][1]
            AND (m)-[:PDG*1.. {{pdgType:"Function", label:m.label}}]->(r)
            
        WITH * 
        MATCH (rSize)-[rSizeEdge:PDG {{pdgType:"Const"}}]->(r)
        WHERE rSizeEdge.value_i >= mSizeEdge.value_i

        RETURN  f.name AS function, mSizeEdge.value_i AS expected_size, rSizeEdge.value_i AS read_size;
        """.format(boFuncs, mallocs)

        queryExecution = session.run(query)

        print("function,expected_size,read_size")
        for (function,expected_size,read_size) in queryExecution:
            print("{0},{1},{2}".format(function,expected_size,read_size))

searchForMallocBufferOverflows()