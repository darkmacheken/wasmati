#!/bin/bash
DEBUG=false

if [[ -z $1 ]]; then
    echo "$0: Required path containing graph files"
    exit 4
fi

# Make sure this is not in history because of password in .config
unset HISTFILE

# Load config file
. .config

GRAPH_DIR_PATH=$(realpath $1)
GRAPH_DIR_BASE=$(basename "$GRAPH_DIR_PATH")
PARENT_DIR=$(dirname "$GRAPH_DIR_PATH")
echo "[INFO] - Reading $GRAPH_DIR_BASE"

# Check if is zip file
ZIP_FILE=false
if [[ $GRAPH_DIR_BASE =~ \.zip$ ]]; then
    ZIP_FILE=true
    GRAPH_DIR_BASE=$(echo $GRAPH_DIR_BASE | cut -d '.' -f1)
    unzip $GRAPH_DIR_PATH -d $PARENT_DIR/$GRAPH_DIR_BASE
    GRAPH_DIR_PATH=$PARENT_DIR/$GRAPH_DIR_BASE
fi

NEO4J_QUERIES=$(realpath ./neo4j_queries)
echo "[INFO] - Will import queries in $NEO4J_QUERIES"

NEO4J_WASMATI_CONTAINER=neo4j-wasmati

RESULTS_DIR=execution-results

# Make sure we have permissions to use graph files (auto chowned by docker...)
echo $password | sudo -S chown $UID:$UID -R $GRAPH_DIR_PATH
echo $password | sudo -S chown $UID:$UID -R $NEO4J_QUERIES

# Build and Run container
echo "[INFO] - Building image for container $NEO4j_WASMATI_CONTAINER"
docker build -q . -t neo4j-docker

if [ "$DEBUG" = true ]; then
    echo "[INFO] - Running container $NEO4j_WASMATI_CONTAINER"
    docker run --rm --name $NEO4J_WASMATI_CONTAINER -v $GRAPH_DIR_PATH:/var/lib/neo4j/import \
        -v $NEO4J_QUERIES:/var/lib/neo4j/queries \
        -e NEO4J_apoc_export_file_enabled=true \
        -e NEO4J_apoc_import_file_enabled=true \
        -e NEO4J_apoc_import_file_use__neo4j__config=true \
        -e NEO4JLABS_PLUGINS=\[\"apoc\"\] \
        -p 7474:7474 -p 7687:7687 neo4j-docker
else
    echo "[INFO] - Running container $NEO4j_WASMATI_CONTAINER"
    docker run -d --rm --name $NEO4J_WASMATI_CONTAINER -v $GRAPH_DIR_PATH:/var/lib/neo4j/import \
        -v $NEO4J_QUERIES:/var/lib/neo4j/queries \
        -e NEO4J_apoc_export_file_enabled=true \
        -e NEO4J_apoc_import_file_enabled=true \
        -e NEO4J_apoc_import_file_use__neo4j__config=true \
        -e NEO4JLABS_PLUGINS=\[\"apoc\"\] \
        -p 7474:7474 -p 7687:7687 neo4j-docker

    echo "[INFO] - Waiting for server to start..."
    # Check if server already accepting connections
    (echo $password | sudo -S tail -f -n0 `docker inspect --format='{{.LogPath}}' $NEO4J_WASMATI_CONTAINER` &) | grep -q "Started."
    echo "[INFO] - Neo4j server started..."

    # RUN queries
    echo "[INFO] - Running queries..."
    docker exec $NEO4J_WASMATI_CONTAINER /var/lib/neo4j/run_queries.sh

    # Stop container
    echo "[INFO] - Stopping and removing container $NEO4j_WASMATI_CONTAINER"
    docker stop $NEO4J_WASMATI_CONTAINER
fi

# Make sure we have permissions to use graph files (auto chowned by docker...)
echo $password | sudo -S chown $UID:$UID -R $GRAPH_DIR_PATH
echo $password | sudo -S chown $UID:$UID -R $NEO4J_QUERIES

if [ "$DEBUG" = false ]; then
# Move results and times to execution results directory
mkdir -p $RESULTS_DIR/$GRAPH_DIR_BASE/
mv $GRAPH_DIR_PATH/*.txt $RESULTS_DIR/$GRAPH_DIR_BASE/
mv $GRAPH_DIR_PATH/*_result.csv $RESULTS_DIR/$GRAPH_DIR_BASE/
fi

# Remove dir if was zip
if [ "$ZIP_FILE" = true ]; then
    rm -r $GRAPH_DIR_PATH
fi
