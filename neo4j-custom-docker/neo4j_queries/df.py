from neo4j import GraphDatabase
import json
import os

driver = GraphDatabase.driver("bolt://localhost:7687/")
session = driver.session()

def searchForDoubleFrees():
    configFile = os.path.join(os.path.dirname(__file__), "defaultConfigs.json")
    with open(configFile, "r") as configs:
        controlFlowPairs = json.load(configs)["controlFlow"]

        mallocs = []
        frees = []

        for pair in controlFlowPairs:
            mallocs.append(pair["source"])
            frees.append(pair["dest"])

        query="""
        WITH {0} as mallocs, {1} as frees

        MATCH (f:Function)-[:AST*1..]->(i:Instruction)
        WHERE i.instType="Call" AND i.label IN mallocs

        WITH *
        MATCH (i)-[:CFG*..]->(free:Instruction)
        WHERE free.instType="Call" AND free.label IN frees AND
            (i)-[:PDG*1.. {{pdgType:"Function", label:i.label}}]->(free)
            
        WITH *
        MATCH (free)-[:CFG*1..]->(uaf:Instruction)
        WHERE uaf.instType="Call" AND uaf.label IN frees AND
            (i)-[:PDG*1.. {{pdgType:"Function", label:i.label}}]->(uaf)    

        RETURN uaf.label as caller, f.name as function;
        """.format(mallocs, frees)
        
        queryExecution = session.run(query)

        print("caller,function")
        for (caller, function) in queryExecution:
            print("{0},{1}".format(caller, function))

searchForDoubleFrees()