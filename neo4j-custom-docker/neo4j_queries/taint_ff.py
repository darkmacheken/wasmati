from neo4j import GraphDatabase
import json
import os

driver = GraphDatabase.driver("bolt://localhost:7687/")
session = driver.session()

def searchForTaintedF2F():
    configFile = os.path.join(os.path.dirname(__file__), "defaultConfigs.json")
    with open(configFile, "r") as configs:
        configurations = json.load(configs)
        sources = configurations["sources"]
        sinks = configurations["sinks"]

        query="""
        WITH {0} as sinks, {1} as sources

        MATCH (f:Function)-[:AST*1..]->(sink_call:Instruction),(source_call:Instruction)
        WHERE sink_call.instType="Call"
            AND sink_call.label IN sinks

        WITH *
        MATCH (source_call:Instruction)-[:PDG*1..]->(sink_call)
        WHERE source_call.instType="Call"
            AND source_call.label IN sources
            AND (source_call)-[:PDG*1.. {{pdgType:"Function", label:source_call.label}}]->(sink_call)

        RETURN source_call.label as source, sink_call.label as sink, f.name as function;
        """.format(sinks, sources)
        
        queryExecution = session.run(query)

        print("source,sink,function")
        for (source,sink,function) in queryExecution:
            print("{0},{1},{2}".format(source,sink,function))

searchForTaintedF2F()