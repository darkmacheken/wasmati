package wasmati;

import org.neo4j.procedure.*;

import java.io.*;
import java.util.*;

/**
 * This is an example how you can create a simple user-defined function for Neo4j.
 */
public class GetBufferLocationMap {

    @UserAggregationFunction("wasmati.getBufferLocationMap")
    @Description("wasmati.getBufferLocationMap(value)")
    public GetBufferLocationMapFunction getBufferLocationMap() {
        return new GetBufferLocationMapFunction();
    }


    public static class GetBufferLocationMapFunction {

        private List<Long> valueList;

        public GetBufferLocationMapFunction() {
            this.valueList = new ArrayList<Long>();
            this.valueList.add(0L);
        }

        private Map<String,Object> toMap(List<Long> buffers) {
            Map<String,Object> map = new HashMap<>();
            
            int length = buffers.size();
            for (int i = 0; i < length - 1; i++) {
                map.put("@"+buffers.get(i), buffers.get(i+1) - buffers.get(i));
            }

            return map;
        }

        @UserAggregationUpdate
        public void aggregate(@Name("value") Long value) {
            if (value != null) {
                this.valueList.add(new Long(value));
            }
        }

        @UserAggregationResult
        public Map<String,Object> result() {
            Collections.sort(this.valueList);
            return this.toMap(new ArrayList<>(new HashSet<>(this.valueList)));
        }
    }
}