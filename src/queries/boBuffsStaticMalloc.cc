#include "src/query.h"
#include "src/vulns.h"
using namespace wasmati;

void VulnerabilityChecker::BOBuffsStaticMalloc() {
    auto start = std::chrono::high_resolution_clock::now();
    json boConfig = config.at(BUFFER_OVERFLOW);
    std::set<std::string> funcSink;

    for (auto const& kv : boConfig.items()) {
        funcSink.insert(kv.key());
    }

    std::set<std::string> ignore = config[IGNORE];

    Index counter = 0;
    for (auto func : Query::functions()) {
        debug("[DEBUG][Query::BoBuffsStatic][%u/%u] Function %s\n", counter++,
              numFuncs, func->name().c_str());
        if (ignore.count(func->name())) {
            continue;
        }

        auto callsPredicate = Predicate()
                                  .instType(ExprType::Call)
                                  .TEST(funcSink.count(node->label()) == 1);
        NodeStream(func).instructions(callsPredicate).forEach([&](Node* call) {
            auto buffer = call->getChild(boConfig.at(call->label()).at(BUFFER));
            auto sizeArg = call->getChild(boConfig.at(call->label()).at(SIZE));
            auto bufferSize = verifyMallocConst(buffer);
            if (!bufferSize.first) {
                return;
            }
            // Get size argument to compare
            Index sizeToWrite = 0;
            bool hasConst = Predicate()
                                .pdgConstEdgeU32(sizeToWrite, false)
                                .evaluate(sizeArg);
            if (!hasConst) {
                return;
            }
            if (bufferSize.second < sizeToWrite) {
                std::string desc =
                    "buffer is " + std::to_string(bufferSize.second) +
                    " and is expecting " + std::to_string(sizeToWrite);
                vulns.emplace_back(VulnType::BufferOverflow, func->name(),
                                   call->label(), desc);
            }
        });
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
        .count();
    info["boBuffsStaticMalloc"] = time;
}
