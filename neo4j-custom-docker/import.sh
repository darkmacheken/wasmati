#!/bin/bash

IMPORT_DIR="/var/lib/neo4j/import"
IMPORT_HEADERS="/var/lib/neo4j/import_headers"

# neo4j-admin import --ignore-empty-strings=true --nodes="${IMPORT_DIR}/node_headers.csv,${IMPORT_DIR}/node.zip" \
# --relationships="${IMPORT_DIR}/edge_headers.csv,${IMPORT_DIR}/edge.zip"

/usr/bin/time -p -o ${IMPORT_DIR}/import_time.txt neo4j-admin import --ignore-empty-strings=true --nodes="${IMPORT_HEADERS}/node_headers.csv,${IMPORT_DIR}/nodes.csv" \
--relationships="${IMPORT_HEADERS}/edge_headers.csv,${IMPORT_DIR}/edges.csv"