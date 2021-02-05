#!/bin/bash

IMPORT_DIR="/var/lib/neo4j/import"

# echo "  Running Cypher queries..."
# CYPHER_QUERIES=$(ls ./queries/*.cypher)
# for f in $CYPHER_QUERIES; do
#     filename_full=$(echo $f | cut -d '/' -f3)
#     filename=$(echo $filename_full | cut -d '.' -f1)
#     /usr/bin/time -p -o ${IMPORT_DIR}/${filename}_time.txt cypher-shell -f $f > ${IMPORT_DIR}/${filename}_result.csv
# done

echo "  Running Python query..."
PYTHON_QUERIES=$(ls ./queries/*.py)
for f in $PYTHON_QUERIES; do
    filename_full=$(echo $f | cut -d '/' -f3)
    filename=$(echo $filename_full | cut -d '.' -f1)
    /usr/bin/timeout --preserve-status 10m /usr/bin/time -p -o ${IMPORT_DIR}/${filename}_time.txt python3 $f > ${IMPORT_DIR}/${filename}_result.csv
done