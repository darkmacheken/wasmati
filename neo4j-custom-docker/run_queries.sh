#!/bin/bash

IMPORT_DIR="/var/lib/neo4j/import"

QUERIES=$(ls ./queries/*.cypher)
for f in $QUERIES; do
    filename_full=$(echo $f | cut -d '/' -f3)
    filename=$(echo $filename_full | cut -d '.' -f1)
    /usr/bin/time -p -o ${IMPORT_DIR}/${filename}_time.txt cypher-shell -f $f > ${IMPORT_DIR}/${filename}_result.csv
done