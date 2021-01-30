package wasmati;

import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.TestInstance;
import org.neo4j.driver.Config;
import org.neo4j.driver.Driver;
import org.neo4j.driver.GraphDatabase;
import org.neo4j.driver.Session;
import org.neo4j.harness.Neo4j;
import org.neo4j.harness.Neo4jBuilders;

import java.util.*;

import static org.assertj.core.api.Assertions.assertThat;

@TestInstance(TestInstance.Lifecycle.PER_CLASS)
public class GetBufferLocationMapTest {

    private static final Config driverConfig = Config.builder().withoutEncryption().build();
    private Neo4j embeddedDatabaseServer;

    @BeforeAll
    void initializeNeo4j() {
        this.embeddedDatabaseServer = Neo4jBuilders
                .newInProcessBuilder()
                .withDisabledServer()
                .withAggregationFunction(GetBufferLocationMap.class)
                .build();
    }


    @Test
    public void shouldReturnBufferLocationMap() {

        // This is in a try-block, to make sure we close the driver after the test
        try(Driver driver = GraphDatabase.driver(embeddedDatabaseServer.boltURI(), driverConfig);
            Session session = driver.session()) {

            Map<String,Object> expected = new HashMap<>();
            expected.put("@0", 16L);
            expected.put("@16", 16L);
            expected.put("@32", 64L);
            expected.put("@96", 64L);

            // When
            Map<String,Object> result = session.run( "UNWIND [16,32,96,160] as value RETURN wasmati.getBufferLocationMap(value) AS map").single().get("map").asMap();

            // Then
            assertThat(result).isEqualTo( expected );
        }
    }

    @Test
    public void shouldReturnBufferLocationMapWithoutDuplicates() {

        // This is in a try-block, to make sure we close the driver after the test
        try(Driver driver = GraphDatabase.driver(embeddedDatabaseServer.boltURI(), driverConfig);
            Session session = driver.session()) {

            Map<String,Object> expected = new HashMap<>();
            expected.put("@0", 16L);
            expected.put("@16", 16L);
            expected.put("@32", 64L);
            expected.put("@96", 64L);

            // When
            Map<String,Object> result = session.run( "UNWIND [32,16,32,96,160] as value RETURN wasmati.getBufferLocationMap(value) AS map").single().get("map").asMap();

            // Then
            assertThat(result).isEqualTo( expected );
        }
    }
}