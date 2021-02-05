# Neo4j Implementation of Wasmati Queries

## Running all tests

You can run the script `run_all.sh` that unzips all zip archives in the directory provided as input and runs the queries for each of these graphs:

```bash
$ run_all.sh <graphs-directory>
```

## Running tests for a specific graph

You can run the script `run_neo4j.sh` that unzips the zip archive provided as input (or uses a directory if the input is not a zip) and runs the queries for this graph:

```bash
$ run_neo4j.sh <graph-zip | graph-directory>
```

## Checking the results

The results are stored in a directory called *execution-results*. Inside this directory there will be a directory for each of the graphs tested. The name of this directory matches the name of the graph zip. Inside this directory you will have three types of files:

1. An *import_time.txt* file, that has the import time of the last execution in the standard format of the *time* utility;

2. Several *\*_time.txt* files, that have the execution time of a particular query for this graph in the standard format of the *time* utility;

3. Several *\*_results.csv* files, that have the execution result of a particular query for this graph in CSV format.

## Troubleshooting

Sometime the neo4j docker blocks. If you suspect the execution is blocked run the following command:

```bash 
$ docker logs neo4j-wasmati
```

This will show you the log of the execution inside the container. If the last line matches with *Fetching versions.json for Plugin...* I suggest you run the following commands:

```bash
$ docker stop neo4j-wasmati
$ sudo service restart docker
```

After this you should be able to run the execution again with no problem.