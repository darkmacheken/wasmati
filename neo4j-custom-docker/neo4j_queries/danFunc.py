from neo4j import GraphDatabase
import json
import os

driver = GraphDatabase.driver("bolt://localhost:7687/")
session = driver.session()

def searchForDangerousFunctions():
    configFile = os.path.join(os.path.dirname(__file__), "defaultConfigs.json")
    with open(configFile, "r") as configs:
        dangerousFuncs = json.load(configs)["dangerousFunctions"]

        query="""
        WITH {0} as DANGEROUS_FUNC
        MATCH (f:Function)-[:AST*1..]->(i:Instruction)
        WHERE i.instType="Call" AND i.label IN DANGEROUS_FUNC
        RETURN i.label as caller,f.name as function;
        """.format(dangerousFuncs)

        queryExecution = session.run(query)

        print("caller,function")
        for (caller, function) in queryExecution:
            print("{0},{1}".format(caller, function))

searchForDangerousFunctions()