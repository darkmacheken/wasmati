from neo4j import GraphDatabase
import json
import os

driver = GraphDatabase.driver("bolt://localhost:7687/")
session = driver.session()

def searchForFormatStrings():
    configFile = os.path.join(os.path.dirname(__file__), "defaultConfigs.json")
    with open(configFile, "r") as configs:
        formatStringDict = json.load(configs)["formatString"]

        formatStrings=list(map(list, formatStringDict.items()))

        query="""
        WITH apoc.map.fromPairs({0}) as formatStrings

        MATCH (f:Function)-[:AST*1..]->(i:Instruction)-[r:AST]->(child)
        WHERE NOT (child)-[:PDG {{pdgType:"Const"}}]->(i) AND
            i.instType="Call" AND i.label IN keys(formatStrings) AND
            r.arg=formatStrings[i.label]
        RETURN i.label as caller,f.name as function;
        """.format(formatStrings)

        queryExecution = session.run(query)

        print("caller,function")
        for (caller, function) in queryExecution:
            print("{0},{1}".format(caller, function))

searchForFormatStrings()