#ifndef WASMATI_VULNS_BUILDER_H_
#define WASMATI_VULNS_BUILDER_H_
#define COMMA ,

#include <fstream>
#include <list>
#include <map>
#include <sstream>
#include "query.h"

#define IMPORT_AS_SOURCES "importAsSources"
#define IMPORT_AS_SINKS "importAsSinks"
#define EXPORTED_AS_SINKS "exportedAsSinks"
#define IGNORE "ignore"
#define WHITELIST "whiteList"
#define SOURCES "sources"
#define SINKS "sinks"
#define TAINTED "tainted"
#define PARAMS "params"
#define BUFFER_OVERFLOW "bufferOverflow"
#define BO_MEMCPY "boMemcpy"
#define DANGEROUS_FUNCTIONS "dangerousFunctions"
#define BUFFER "buffer"
#define SIZE "size"
#define FORMAT_STRING "formatString"
#define MALLOC "malloc"
#define CONTROL_FLOW "controlFlow"
#define SOURCE "source"
#define DEST "dest"

namespace wasmati {

static const json defaultConfig = R"(
{
	"importAsSources": true,
	"importAsSinks": true,
	"exportedAsSinks": true,
	"ignore": [
		"$fgets",
		"$__stdio_read",
		"$__stdio_write",
		"$__stdio_seek",
		"$__fwritex",
		"$exit",
		"$dlmalloc",
		"$dlfree",
		"$dlrealloc",
		"$setenv",
		"$decfloat",
		"$sn_write",
		"$sbrk",
		"$__strdup",
		"$__expand_heap",
		"$raise",
		"$emscripten_memcpy_big",
		"$emscripten_resize_heap"
	],
	"whiteList": [
		"$__wasi_fd_close",
		"$env._emval_decref",
		"$env.testSetjmp",
		"$wasi_snapshot_preview1.fd_close",
		"$env.invoke_iii",
		"$env.emscripten_longjmp",
		"$env.setTempRet0",
		"$wasi_snapshot_preview1.fd_seek",
		"$wasi_snapshot_preview1.fd_read",
		"$wasi_snapshot_preview1.fd_write",
		"$env.emscripten_resize_heap",
		"$env.emscripten_memcpy_big",
		"$fflush"
	],
	"sources": [ "$read_bytes_to_mmap_memory" ],
	"sinks": [],
	"tainted": {
		"$main": { "params": [ 0, 1 ] },
		"$store_data": { "params": [ 0 ] },
		"$very_complex_function": { "params": [ 0 ] }
	},
	"bufferOverflow": {
		"$read": {
			"buffer": 1,
			"size": 2
		},
		"$fgets": {
			"buffer": 0,
			"size": 1
		}
	},
	"boMemcpy": [ "$strcpy", "$__stpcpy", "$memcpy" ],
	"dangerousFunctions": [ "$gets", "$strcat" ],
	"formatString": {
		"$fprintf": 1,
		"$printf": 0,
		"$iprintf": 0,
		"$sprintf": 1,
		"$snprintf": 2,
		"$vfprintf": 1,
		"$vprintf": 0,
		"$vsprintf": 1,
		"$vsnprintf": 2,
		"$syslog": 1,
		"$vsyslog": 1
	},
	"malloc": [ "$malloc", "$dlmalloc" ],
	"controlFlow": [
		{
			"source": "$malloc",
			"dest": "$free"
		},
		{
			"source": "$dlmalloc",
			"dest": "$dlfree"
		}
	]
}
)"_json;

enum class VulnType {
    Unreachable,
    DangFunc,
    FormatStrings,
    BufferOverflow,
    Tainted,
    UaF,
    DoubleFree,
    IntegerOverflow
};

NLOHMANN_JSON_SERIALIZE_ENUM(VulnType,
                             {
                                 {VulnType::Unreachable, "Unreachable"},
                                 {VulnType::DangFunc, "Dangerous Function"},
                                 {VulnType::FormatStrings, "Format Strings"},
                                 {VulnType::BufferOverflow, "Buffer Overflow"},
                                 {VulnType::Tainted, "Tainted Variable"},
                                 {VulnType::UaF, "Use After Free"},
                                 {VulnType::DoubleFree, "Double Free"},
                                 {VulnType::IntegerOverflow,
                                  "Integer Overflow"},
                             });

struct Vulnerability {
    VulnType type;
    std::string function;
    std::string caller;
    std::string description;
    json object;
    bool is_object = false;

    Vulnerability(VulnType type,
                  const std::string& function,
                  const std::string& caller,
                  const std::string description = "")
        : type(type),
          function(function),
          caller(caller),
          description(description) {}

    Vulnerability(VulnType type, json object)
        : type(type), object(object), is_object(true) {
        assert(object.contains("function"));
        assert(object.contains("caller"));
    }

    friend void to_json(json& j, const Vulnerability& v) {
        if (v.is_object) {
            j = v.object;
            return;
        }
        j = json{{"type", v.type}, {"function", v.function}};
        if (!v.caller.empty()) {
            j["caller"] = v.caller;
        }
        if (!v.description.empty()) {
            j["description"] = v.description;
        }
    }

    friend void from_json(const json& j, Vulnerability& v) {
        v.object = j;
        v.is_object = true;
    }
};

struct VulnerabilityChecker {
    Index numFuncs;
    const json& config;
    std::list<Vulnerability>& vulns;

    VulnerabilityChecker(json& config, std::list<Vulnerability>& vulns)
        : config(config), vulns(vulns) {
        verifyConfig(config);
    }

    void checkVulnerabilities() {
        numFuncs = Query::functions().size();

#define WASMATI_QUERY(FunctionName, Name, Description) FunctionName();
#include "src/queries/queries.def"
#undef WASMATI_QUERY
    }

#define WASMATI_QUERY(FunctionName, Name, Description) void FunctionName();
#include "src/queries/queries.def"
#undef WASMATI_QUERY

#define WASMATI_QUERY_AUX(FunctionName, ReturnType, ...) \
    ReturnType FunctionName(__VA_ARGS__);
#include "src/queries/queries.def"
#undef WASMATI_QUERY_AUX

    static void verifyConfig(const json& config) {
        // importAsSources
        assert(config.contains(IMPORT_AS_SOURCES));
        assert(config.at(IMPORT_AS_SOURCES).is_boolean());
        // importAsSinks
        assert(config.contains(IMPORT_AS_SINKS));
        assert(config.at(IMPORT_AS_SINKS).is_boolean());
        // exportedAsSinks
        assert(config.contains(EXPORTED_AS_SINKS));
        assert(config.at(EXPORTED_AS_SINKS).is_boolean());
        // blackList
        assert(config.contains(IGNORE));
        assert(config.at(IGNORE).is_array());
        for (auto const& item : config.at(IGNORE).items()) {
            assert(item.value().is_string());
        }
        // whiteList
        assert(config.contains(WHITELIST));
        assert(config.at(WHITELIST).is_array());
        for (auto const& item : config.at(WHITELIST).items()) {
            assert(item.value().is_string());
        }
        // sources
        assert(config.contains(SOURCES));
        assert(config.at(SOURCES).is_array());
        for (auto const& item : config.at(SOURCES).items()) {
            assert(item.value().is_string());
        }
        // sinks
        assert(config.contains(SINKS));
        assert(config.at(SINKS).is_array());
        for (auto const& item : config.at(SINKS).items()) {
            assert(item.value().is_string());
        }
        // tainted
        assert(config.contains(TAINTED));
        assert(config.at(TAINTED).is_object());
        for (auto const& item : config.at(TAINTED).items()) {
            assert(item.value().is_object());
            assert(item.value().contains(PARAMS));
            assert(item.value().at(PARAMS).is_array());
            for (auto const& param : item.value().at(PARAMS).items()) {
                assert(param.value().is_number_integer());
                assert(param.value() >= 0);
            }
        }
        // bufferOverflow
        assert(config.contains(BUFFER_OVERFLOW));
        assert(config.at(BUFFER_OVERFLOW).is_object());
        for (auto const& item : config.at(BUFFER_OVERFLOW).items()) {
            assert(item.value().is_object());
            assert(item.value().contains(BUFFER));
            assert(item.value().at(BUFFER).is_number_integer());
            assert(item.value().at(BUFFER) >= 0);
            assert(item.value().contains(SIZE));
            assert(item.value().at(SIZE).is_number_integer());
            assert(item.value().at(SIZE) >= 0);
        }
        // boMemcpy
        assert(config.contains(BO_MEMCPY));
        assert(config.at(BO_MEMCPY).is_array());
        for (auto const& param : config.at(BO_MEMCPY).items()) {
            assert(param.value().is_string());
        }

        // dangerousFunctions
        assert(config.contains(DANGEROUS_FUNCTIONS));
        assert(config.at(DANGEROUS_FUNCTIONS).is_array());
        for (auto const& item : config.at(DANGEROUS_FUNCTIONS).items()) {
            assert(item.value().is_string());
        }

        // formatString
        assert(config.contains(FORMAT_STRING));
        assert(config.at(FORMAT_STRING).is_object());
        for (auto const& item : config.at(FORMAT_STRING).items()) {
            assert(item.value().is_number_integer());
            assert(item.value() >= 0);
        }

        // malloc
        assert(config.contains(MALLOC));
        assert(config.at(MALLOC).is_array());
        for (auto const& item : config.at(MALLOC).items()) {
            assert(item.value().is_string());
        }

        // constrolFlow
        assert(config.contains(CONTROL_FLOW));
        assert(config.at(CONTROL_FLOW).is_array());
        for (auto const& item : config.at(CONTROL_FLOW)) {
            assert(item.is_object());
            assert(item.contains(SOURCE));
            assert(item.contains(DEST));
        }
    }
};
}  // namespace wasmati
#endif  // WABT_AST_BUILDER_H_
