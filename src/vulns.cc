#include "vulns.h"
using namespace wasmati;

void VulnerabilityChecker::verifyConfig(const json& config) {
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
    // constrolFlow
    assert(config.contains(CONTROL_FLOW));
    assert(config.at(CONTROL_FLOW).is_array());
    for (auto const& item : config.at(CONTROL_FLOW)) {
        assert(item.is_object());
        assert(item.contains(SOURCE));
        assert(item.contains(DEST));
    }
}

void VulnerabilityChecker::checkVulnerabilities() {
    numFuncs = Query::functions().size();
    checkUnreachableCode();
    checkBufferOverflow();
    checkFormatString();
    checkTainted();
    checkUseAfterFree();
}

void VulnerabilityChecker::checkUnreachableCode() {
    Index counter = 0;
    NodeStream(Query::functions()).forEach([&](Node* func) {
        debug("[DEBUG][Query::UnreachableCode][%u/%u] Function %s\n", counter++,
              numFuncs, func->name().c_str());
        auto queryInsts = Query::instructions({func}, [](Node* node) {
            return node->instType() != ExprType::Return &&
                   node->instType() != ExprType::Block &&
                   node->instType() != ExprType::Loop &&
                   node->instType() != ExprType::Unreachable &&
                   node->inEdges(EdgeType::CFG).size() == 0;
        });
        if (queryInsts.size() > 0) {
            vulns.emplace_back(VulnType::Unreachable, func->name(), "", "");
        }
    });
}

void VulnerabilityChecker::checkBufferOverflow() {
    checkBoBuffsStatic();
    checkBoScanfLoops();
}

void VulnerabilityChecker::checkBoBuffsStatic() {
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
                .instType(ExprType::Call)
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
                                         .instType(ExprType::Binary)
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

        // Dangerous functions
        std::set<std::string> dangFuncs(config.at(DANGEROUS_FUNCTIONS).begin(),
                                        config.at(DANGEROUS_FUNCTIONS).end());
        auto dangFunc = Predicate()
                            .instType(ExprType::Call)
                            .TEST(dangFuncs.count(node->label()) == 1);

        NodeStream(func).instructions(dangFunc).forEach([&](Node* node) {
            vulns.emplace_back(VulnType::BufferOverflow, func->name(),
                               node->label(), "");
        });
    }
}

void VulnerabilityChecker::checkBoScanfLoops() {
    std::set<std::string> ignore = config[IGNORE];

    Index counter = 0;
    for (auto func : Query::functions()) {
        debug("[DEBUG][Query::BoScanfLoops][%u/%u] Function %s\n", counter++,
              numFuncs, func->name().c_str());
        if (ignore.count(func->name())) {
            continue;
        }
        // find all loops
        auto loops = Query::instructions({func}, [](Node* node) {
            return node->instType() == ExprType::Loop;
        });

        for (Node* loop : loops) {
            auto insts = Query::BFS({loop}, Query::ALL_INSTS, Query::AST_EDGES);
            auto callScanf = Query::filter(insts, [](Node* node) {
                return node->instType() == ExprType::Call &&
                       node->label() == "$scanf" &&
                       node->getChild(1)->instType() == ExprType::LocalGet;
            });
            if (callScanf.size() == 0) {
                continue;
            }

            // Get the variables dependences on arg 1
            std::set<std::string> varDepend;
            for (Node* call : callScanf) {
                Node* param1 = call->getChild(1);
                auto outEdges = param1->outEdges(EdgeType::PDG);
                for (Edge* e : outEdges) {
                    if (e->pdgType() == PDGType::Global ||
                        e->pdgType() == PDGType::Local) {
                        varDepend.insert(e->label());
                    }
                }
            }

            // find br_if
            auto brifs = Query::filter(insts, [&](Node* node) {
                if (node->instType() == ExprType::BrIf &&
                    node->label() == loop->label()) {
                    Node* child = node->getChild(0);
                    return child->instType() == ExprType::Compare &&
                           child->opcode() != Opcode::I32Eq &&
                           child->opcode() != Opcode::I32Eqz;
                }
                return false;
            });
            if (brifs.size() == 0) {
                continue;
            }

            // check if there is a load
            for (Node* brif : brifs) {
                auto instsBrif =
                    Query::BFS({brif}, Query::ALL_NODES, Query::AST_EDGES);
                auto loadInsts = Query::filter(instsBrif, [&](Node* node) {
                    return node->instType() == ExprType::Load &&
                           Query::containsEdge(
                               node->inEdges(EdgeType::PDG), [&](Edge* e) {
                                   return varDepend.count(e->label()) == 1;
                               });
                });
                if (loadInsts.size() > 0) {
                    std::stringstream desc;
                    Node* childLoad = (*loadInsts.begin())->getChild(0);
                    if (childLoad->instType() != ExprType::LocalGet) {
                        return;
                    }
                    Node* param1 = brif->getChild(0);
                    auto inEdges = param1->inEdges(EdgeType::PDG);
                    auto filterEdges = Query::filterEdges(inEdges, [](Edge* e) {
                        return e->pdgType() == PDGType::Const;
                    });
                    if (filterEdges.size() == 0) {
                        continue;
                    }
                    desc << "In loop " << loop->label() << ":";
                    desc << " buffer pointed by " << childLoad->label();
                    desc << " reaches $scanf until *" << childLoad->label()
                         << " = "
                         << std::to_string((*filterEdges.begin())->value().u32);
                    vulns.emplace_back(VulnType::BufferOverflow, func->name(),
                                       "", desc.str());
                }
            }
        }
    }
}

std::pair<Index, std::map<Index, Index>> VulnerabilityChecker::checkBufferSizes(
    Node* func) {
    // Find SUB
    Index val = 0;
    auto subPred = Predicate()
                       .instType(ExprType::Binary)
                       .opcode(Opcode::I32Sub)
                       .inPDGEdge("$g0", PDGType::Global)
                       .pdgConstEdgeU32(val)
                       .TEST(val > 0);

    auto subInst = NodeStream(func).instructions(subPred).findFirst();

    if (!subInst.isPresent()) {
        return std::make_pair(0, std::map<Index, Index>());
    }

    Index sizeAlloc = 0;
    Predicate().pdgConstEdgeU32(sizeAlloc).evaluate(subInst.get());

    // FIND ADDS
    std::set<Index> setBuffs;
    setBuffs.insert(0);
    setBuffs.insert(sizeAlloc);
    auto addPred = Predicate()
                       .instType(ExprType::Binary)
                       .opcode(Opcode::I32Add)
                       .inPDGEdge("$g0", PDGType::Global)
                       .pdgConstEdgeU32(val)
                       .TEST(val > 0)
                       .EXEC(setBuffs.insert(val));
    NodeStream(func).instructions(addPred);

    // Calculate difference
    std::vector<Index> buffs(setBuffs.begin(), setBuffs.end());
    std::adjacent_difference(buffs.begin(), buffs.end(), buffs.begin());

    std::map<Index, Index> result;
    Index i = 1;  // starts at 1 cuz index 0 = 0
    for (Index buff : setBuffs) {
        if (i < buffs.size()) {
            result[buff] = buffs[i];
            i++;
        } else {
            result[buff] = 0;
        }
    }

    return std::make_pair(sizeAlloc, result);
}

void VulnerabilityChecker::checkFormatString() {
    json fsConfig = config.at(FORMAT_STRING);
    std::set<std::string> ignore = config[IGNORE];

    Index counter = 0;
    for (auto func : Query::functions()) {
        debug("[DEBUG][Query::FormatString][%u/%u] Function %s\n", counter++,
              numFuncs, func->name().c_str());
        if (ignore.count(func->name()) || fsConfig.contains(func->name())) {
            continue;
        }

        Node* child = nullptr;
        auto callPredicate =
            Predicate()
                .instType(ExprType::Call)
                .TEST(fsConfig.contains(node->label()))
                .EXEC(child = node->getChild(fsConfig.at(node->label())))
                .PDG_EDGE(child, node, PDGType::Const, false);

        NodeStream(func).instructions(callPredicate).forEach([&](Node* call) {
            // write vulns
            vulns.emplace_back(VulnType::FormatStrings, func->name(),
                               call->label());
        });
    }
}

void VulnerabilityChecker::checkTainted() {
    taintedFuncToFunc();
    taintedLocalToFunc();
}

void VulnerabilityChecker::taintedFuncToFunc() {
    std::set<std::string> sources = config.at(SOURCES);
    std::set<std::string> sinks = config.at(SINKS);

    if (config.at(IMPORT_AS_SOURCES)) {
        auto funcs = Query::map<std::string>(
            Query::functions([](Node* node) { return node->isImport(); }),
            [](Node* node) { return node->name(); });
        sources.insert(funcs.begin(), funcs.end());
    }

    if (config.at(IMPORT_AS_SINKS)) {
        auto funcs = Query::map<std::string>(
            Query::functions([](Node* node) { return node->isImport(); }),
            [](Node* node) { return node->name(); });
        sinks.insert(funcs.begin(), funcs.end());
    }

    std::set<std::string> whitelist = config.at(WHITELIST);
    for (std::string funcName : whitelist) {
        sinks.erase(funcName);
    }

    std::set<std::string> ignore = config[IGNORE];

    Index counter = 0;
    for (auto func : Query::functions()) {
        debug("[DEBUG][Query::TaintedFuncToFunc][%u/%u] Function %s\n",
              counter++, numFuncs, func->name().c_str());
        if (ignore.count(func->name())) {
            continue;
        }
        if (sinks.count(func->name()) == 1) {
            // if it is already a sink, no use check
            continue;
        }
        auto query = Query::instructions({func}, [&](Node* node) {
            if (node->instType() == ExprType::Call &&
                sinks.count(node->label()) == 1) {
                auto pdgEdges = node->inEdges(EdgeType::PDG);
                return Query::containsEdge(pdgEdges, [&](Edge* e) {
                    if (e->pdgType() == PDGType::Function &&
                        sources.count(e->label()) == 1) {
                        std::stringstream desc;
                        desc << "Source " << e->label() << " reaches sink "
                             << node->label();
                        vulns.emplace_back(VulnType::Tainted, func->name(),
                                           node->label(), desc.str());
                        return true;
                    }
                    return false;
                });
            }
            return false;
        });
    }
}

void VulnerabilityChecker::taintedLocalToFunc() {
    std::set<std::string> sinks = config.at(SINKS);

    if (config.at(IMPORT_AS_SINKS)) {
        auto funcs = Query::map<std::string>(
            Query::functions([](Node* node) { return node->isImport(); }),
            [](Node* node) { return node->name(); });
        sinks.insert(funcs.begin(), funcs.end());
    }

    std::set<std::string> whitelist = config.at(WHITELIST);
    for (std::string funcName : whitelist) {
        sinks.erase(funcName);
    }

    std::set<std::string> ignore = config[IGNORE];

    Index counter = 0;
    for (auto func : Query::functions()) {
        debug("[DEBUG][Query::TaintedLocalToFunc][%u/%u] Function %s\n",
              counter++, numFuncs, func->name().c_str());
        if (ignore.count(func->name())) {
            continue;
        }
        if (sinks.count(func->name()) == 1) {
            // if it is already a sink, no use check
            continue;
        }
        std::map<std::string, std::pair<std::string, std::string>>
            taintedParams;
        NodeStream(func).parameters().forEach([&](Node* param) {
            std::set<std::string> visited;
            taintedParams[param->name()] = isTainted(param, visited);
        });

        NodeStream(func)
            .instructions([&](Node* node) {
                return node->instType() == ExprType::Call &&
                       sinks.count(node->label()) == 1;
            })
            .forEach([&](Node* call) {
                auto localsDependsList =
                    EdgeStream(call->inEdges(EdgeType::PDG))
                        .filterPDG(PDGType::Local)
                        .map<std::string>([](Edge* e) { return e->label(); });

                std::set<std::string> localsDepends(localsDependsList.begin(),
                                                    localsDependsList.end());

                NodeStream(call)
                    .children(Query::AST_EDGES)
                    .forEach([&](Node* arg) {
                        auto argsLocalsDepends =
                            EdgeStream(arg->inEdges(EdgeType::PDG))
                                .filterPDG(PDGType::Local)
                                .map<std::string>(
                                    [](Edge* e) { return e->label(); });

                        localsDepends.insert(argsLocalsDepends.begin(),
                                             argsLocalsDepends.end());
                    });

                for (std::string local : localsDepends) {
                    if (taintedParams.count(local) == 0) {
                        continue;
                    } else if (taintedParams[local].first == "") {
                        continue;
                    } else {
                        std::stringstream desc;
                        desc << local << " tainted from param "
                             << taintedParams[local].first << " in "
                             << taintedParams[local].second;
                        vulns.emplace_back(VulnType::Tainted, func->name(),
                                           call->label(), desc.str());
                    }
                }
            });
    }
}

std::pair<std::string, std::string> VulnerabilityChecker::isTainted(
    Node* param,
    std::set<std::string>& visited) {
    Node* func = Query::function(param);
    if (visited.count(func->name()) == 1) {
        return std::make_pair("", "");
    }
    visited.insert(func->name());

    if (config[TAINTED].contains(func->name())) {
        for (Index index : config[TAINTED][func->name()][PARAMS]) {
            if (index == param->index()) {
                return std::make_pair(param->name(), func->name());
            }
        }
    } else if (config[EXPORTED_AS_SINKS] && func->isExport()) {
        std::set<std::string> whitelist = config[WHITELIST];
        if (whitelist.count(func->name()) == 0) {
            return std::make_pair(param->name(), func->name());
        }
    }
    for (auto arg : Query::parents({param}, Query::PG_EDGES)) {
        auto localVarsList =
            EdgeStream(arg->outEdges(EdgeType::PDG))
                .setUnion(arg->inEdges(EdgeType::PDG))
                .filterPDG(PDGType::Local)
                .distincLabel()
                .map<std::string>([](Edge* e) { return e->label(); });
        std::set<std::string> localVars(localVarsList.begin(),
                                        localVarsList.end());

        auto newParams = NodeStream(Query::function(arg))
                             .parameters([&](Node* node) {
                                 return localVars.count(node->name()) == 1;
                             })
                             .toNodeSet();
        for (Node* param : newParams) {
            auto tainted = isTainted(param, visited);
            if (tainted.first != "") {
                return tainted;
            }
        }
    }
    return std::make_pair("", "");
}

void VulnerabilityChecker::checkUseAfterFree() {
    std::set<std::string> ignore = config[IGNORE];

    Index counter = 0;
    for (auto func : Query::functions()) {
        debug("[DEBUG][Query::UseAfterFree][%u/%u] Function %s\n", counter++,
              numFuncs, func->name().c_str());
        if (ignore.count(func->name())) {
            continue;
        }
        for (auto const& item : config[CONTROL_FLOW]) {
            std::string source = item[SOURCE];
            std::string dest = item[DEST];

            NodeStream(func)
                .instructions([&](Node* node) {
                    return node->instType() == ExprType::Call &&
                           node->label() == source;
                })
                .forEach([&](Node* callSource) {
                    Query::DFS<bool>(
                        callSource, Query::CFG_EDGES, false,
                        [&](Node* node, bool seenDest) {
                            auto edges =
                                EdgeStream(node->inEdges(EdgeType::PDG))
                                    .setUnion(node->outEdges(EdgeType::PDG))
                                    .filterPDG(PDGType::Function,
                                               callSource->label())
                                    .findFirst();
                            if (node->type() != NodeType::Instruction) {
                                return std::make_pair(false, seenDest);
                            } else if (node->instType() == ExprType::Call &&
                                       node->label() == dest) {
                                if (seenDest && edges.isPresent()) {
                                    std::stringstream desc;
                                    desc << node->label() << " called again.";
                                    vulns.emplace_back(
                                        VulnType::DoubleFree, func->name(),
                                        node->label(), desc.str());
                                }
                                return std::make_pair(true, edges.isPresent());
                            } else if (seenDest && edges.isPresent()) {
                                std::stringstream desc;
                                desc << "Value from call "
                                     << callSource->label()
                                     << " used after call to " << dest;
                                vulns.emplace_back(VulnType::UaF, func->name(),
                                                   node->label(), desc.str());
                                return std::make_pair(false, seenDest);
                            }
                            return std::make_pair(true, seenDest);
                        });
                });
        }
    }
}
