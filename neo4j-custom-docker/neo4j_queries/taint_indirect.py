from neo4j import GraphDatabase
import json
import os

driver = GraphDatabase.driver("bolt://localhost:7687/")
session = driver.session()

def searchForTaintedIndirect():
    configFile = os.path.join(os.path.dirname(__file__), "defaultConfigs.json")
    with open(configFile, "r") as configs:
        configurations = json.load(configs)
        sources = configurations["sources"]

        query="""
        WITH {0} as sources

        MATCH (f:Function)-[:AST*1..]->(source_call:Instruction)
        WHERE source_call.instType="Call"
            AND source_call.label IN sources
            
        WITH *
        MATCH (call_indirect:Instruction)-[astEdge:AST]->(source_call)
        WHERE call_indirect.instType="CallIndirect"
            AND astEdge.arg=call_indirect.nargs-1
            AND (source_call)-[:PDG*1.. {{pdgType:"Function", label:source_call.label}}]->(call_indirect)

        RETURN source_call.label as source, call_indirect.label as sink, f.name as function;
        """.format(sources)
        
        queryExecution = session.run(query)

        print("source,sink,function")
        for (source,sink,function) in queryExecution:
            print("{0},{1},{2}".format(source,sink,function))

searchForTaintedIndirect()