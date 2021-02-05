from neo4j import GraphDatabase
import json
import os

driver = GraphDatabase.driver("bolt://localhost:7687/")
session = driver.session()

def searchForStaticBufferOverflows():
    configFile = os.path.join(os.path.dirname(__file__), "defaultConfigs.json")
    with open(configFile, "r") as configs:
        configurations = json.load(configs)
        bufferOverflowFunctions = configurations["bufferOverflow"]
        boFuncs = []
        for boFunc, parameters in iter(bufferOverflowFunctions.items()):
            boFuncs.append([boFunc, [parameters["buffer"], parameters["size"]]])

        query="""
        // List of vulnerable functions
        WITH apoc.map.fromPairs({0}) AS boFuncs

        // Get reference to function and call to vulnerable function in list
        MATCH (f:Function)-[:AST*1..]->(call:Instruction)
        WHERE call.instType="Call" AND call.label IN keys(boFuncs)

        // Check if the call has some positional argument
        WITH *
        MATCH (call)-[:AST {{arg:boFuncs[call.label][0]}}]->(buff:Instruction)

        // Get the position argument for the read size
        WITH *
        MATCH (call)-[:AST {{arg:boFuncs[call.label][1]}}]->(sizeRead:Instruction)

        // Get alloc size
        WITH *
        MATCH (f)-[:AST*1..]->(s:Instruction)<-[subPDG:PDG]-()
        WHERE s.instType="Binary"
            AND s.opcode="i32.sub"
            AND subPDG.pdgType="Const"
            AND subPDG.value_type="i32"
            AND subPDG.value_i > 0
            AND ()-[:PDG {{pdgType:"Global", label:"$g0"}}]->(s)

        // Find buffer location
        WITH *
        MATCH (f)-[:AST*1..]->(a:Instruction)<-[addPDG:PDG]-()
        WHERE a.instType="Binary"
            AND a.opcode="i32.add"
            AND addPDG.pdgType="Const"
            AND addPDG.value_type="i32"
            AND addPDG.value_i > 0
            AND (s)-[:PDG*1.. {{pdgType:"Global", label:"$g0"}}]->(a)

        // Create map from location to size
        WITH f, call, sizeRead, wasmati.getBufferLocationMap(addPDG.value_i) AS buffLoc

        // Check if there is a path between an add instruction and the call to dangerous function
        OPTIONAL MATCH addPath=(add:Instruction)-[:PDG*1.. {{pdgType:"Global", label:"$g0"}}]->(call)
        WHERE add.instType="Binary"
            AND add.opcode="i32.add"

        // Get the PDG edge for that add instruction
        WITH *
        OPTIONAL MATCH ()-[locEdge:PDG]->(add)

        // Get the location index implicit in that PDG edge
        WITH *, CASE WHEN addPath is null THEN 0 ELSE locEdge.value_i END AS buffIndex

        // Check if the read size is larger or equal to the size of the corresponding buffer
        WITH *
        MATCH (sizeRead)-[sizePDG:PDG {{pdgType:"Const"}}]->(call)
        WHERE sizePDG.value_i >= buffLoc["@"+buffIndex]

        // Return results
        RETURN f.name AS function, "@"+buffIndex AS buffer_location, buffLoc["@"+buffIndex] AS expected_size, sizePDG.value_i AS read_size;
        """.format(boFuncs)

        queryExecution = session.run(query)

        print("function,buffer_location,expected_size,read_size")
        for (function, buffer_location, expected_size, read_size) in queryExecution:
            print("{0},{1},{2},{3}".format(function, buffer_location, expected_size, read_size))

searchForStaticBufferOverflows()