#include "src/query.h"
#include "src/vulns.h"
using namespace wasmati;

void VulnerabilityChecker::BOBuffsStatic() {
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

        auto buffersSizes = checkBufferSizes(func);
        const Index sizeAlloc = buffersSizes.first;
        const std::map<Index, Index> buffers = buffersSizes.second;

        Node* children = nullptr;
        auto callsPredicate =
            Predicate()
                .instType(InstType::Call)
                .TEST(funcSink.count(node->label()) == 1)
                .EXEC(children =
                          node->getChild(boConfig.at(node->label()).at(BUFFER)))
                .PDG_EDGE(children, node, "$g0", PDGType::Global, true);

        NodeStream(func).instructions(callsPredicate).forEach([&](Node* node) {
            auto bufferArg =
                node->getChild(boConfig.at(node->label()).at(BUFFER));
            auto sizeArg = node->getChild(boConfig.at(node->label()).at(SIZE));

            Index val = 0;
            auto addOrSubPredicate = Predicate()
                                         .instType(InstType::Binary)
                                         .insert(Predicate()
                                                     .opcode(Opcode::I32Add)
                                                     .Or()
                                                     .opcode(Opcode::I32Sub))
                                         .inPDGEdge("$g0", PDGType::Global)
                                         .pdgConstEdgeU32(val)
                                         .TEST(val <= sizeAlloc);

            auto addOrSubInst =
                NodeStream(bufferArg)
                    .BFSincludes(addOrSubPredicate,
                                 Query::pdgEdge("$g0", PDGType::Global), 1,
                                 true)
                    .findFirst();

            if (!addOrSubInst.isPresent()) {
                return;
            }

            Index buff = 0;
            Predicate().pdgConstEdgeU32(buff).evaluate(addOrSubInst.get());

            // If it gets to the SUB inst, buff will be equal to sizeAlloc
            // in this case should point to buffer 0 and size=sizeAlloc
            if (buff == sizeAlloc) {
                buff = 0;
            }

            // Get size argument to compare
            Index sizeToWrite = 0;
            bool hasConst = Predicate()
                                .pdgConstEdgeU32(sizeToWrite, false)
                                .evaluate(sizeArg);
            if (!hasConst) {
                return;
            }
            Index size;
            if (buff < 32 && sizeToWrite > sizeAlloc - buff) {
                size = sizeAlloc - buff;
            } else if (buff >= 32 && sizeToWrite > buffers.at(buff)) {
                size = buffers.at(buff);
            } else {
                return;
            }
            std::string desc = "buffer @+" + std::to_string(buff) + " is " +
                               std::to_string(size) + " and is expecting " +
                               std::to_string(sizeToWrite);
            vulns.emplace_back(VulnType::BufferOverflow, func->name(),
                               node->label(), desc);
        });
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
    info["boBuffsStatic"] = time;
}
