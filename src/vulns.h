#ifndef WASMATI_VULNS_BUILDER_H_
#define WASMATI_VULNS_BUILDER_H_

#include <list>
#include <map>
#include <sstream>
#include "query.h"

#define IMPORT_AS_SOURCES  "importAsSources"
#define EXPORTED_AS_SINKS  "exportedAsSinks"
#define BLACKLIST  "blackList"
#define WHITELIST  "whiteList"
#define SOURCES  "sources"
#define SINKS  "sinks"
#define TAINTED  "tainted"
#define PARAMS  "params"
#define BUFFER_OVERFLOW  "bufferOverflow"
#define BUFFER  "buffer"
#define SIZE  "size"
#define FORMAT_STRING  "formatString"

namespace wasmati {


static const json defaultConfig = R"(
{
	"importAsSources": true,
	"exportedAsSinks": true,
	"blackList": [],
	"whiteList": [],
    "sources": [],
    "sinks": [],
	"tainted": {
		"main": { "params": [ 0, 1 ] }
	},
	"bufferOverflow": {
		"$read": {
			"buffer": 1,
			"size": 2
		},
		"$fgets": {
			"buffer": 0,
			"size": 1
		},
		"$gets": {
			"buffer": 0
		}
	},
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
	}
}
)"_json;

enum class VulnType {
    Unreachable,
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
                                 {VulnType::FormatStrings, "Format Strings"},
                                 {VulnType::BufferOverflow, "Buffer Overflow"},
                                 {VulnType::Tainted, "Tainted Variable"},
                                 {VulnType::UaF, "Use After Free"},
                                 {VulnType::DoubleFree, "Double Free"},
                                 {VulnType::IntegerOverflow, "IntegerOverflow"},
                             });

struct Vulnerability {
    VulnType type;
    std::string function;
    std::string caller;
    std::string description;

    Vulnerability(VulnType type,
                  const std::string& function,
                  const std::string& caller,
                  const std::string description = "")
        : type(type),
          function(function),
          caller(caller),
          description(description) {}

    friend void to_json(json& j, const Vulnerability& v) {
        j = json{{"type", v.type}, {"function", v.function}};
        if (!v.caller.empty()) {
            j["caller"] = v.caller;
        }
        if (!v.description.empty()) {
            j["description"] = v.description;
        }
    }

    friend void from_json(const json& j, Vulnerability& v) {
        j.at("type").get_to(v.type);
        j.at("function").get_to(v.function);
        if (j.contains("caller")) {
            j.at("caller").get_to(v.caller);
        }
        if (j.contains("description")) {
            j.at("description").get_to(v.description);
        }
    }
};

void checkVulnerabilities(json& config, std::list<Vulnerability>& vulns);
void verifyConfig(json& config);

void checkUnreachableCode(json& config, std::list<Vulnerability>& vulns);

void checkBufferOverflow(json& config, std::list<Vulnerability>& vulns);
void checkBoBuffsStatic(json& config, std::list<Vulnerability>& vulns);
void checkBoScanfLoops(json& config, std::list<Vulnerability>& vulns);

void checkFormatString(json& config, std::list<Vulnerability>& vulns);
void checkTainted(json& config, std::list<Vulnerability>& vulns);
void checkIntegerOverflow(json& config);
void checkUseAfterFree(json& config);
std::map<int, int> checkBufferSizes(Node* func);

}  // namespace wasmati
#endif  // WABT_AST_BUILDER_H_
