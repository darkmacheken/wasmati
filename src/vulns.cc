#include "vulns.h"
using namespace wasmati;

void wasmati::checkVulnerabilities(json& config,
                                   std::list<Vulnerability>& vulns) {
    verifyConfig(config);
    checkUnreachableCode(config, vulns);
    checkBufferOverflow(config, vulns);
    checkFormatString(config, vulns);
    checkTainted(config, vulns);
    // checkIntegerOverflow();
    // checkUseAfterFree();
}

void wasmati::verifyConfig(json& config) {
    // importAsSources
    assert(config.contains(IMPORT_AS_SOURCES));
    assert(config.at(IMPORT_AS_SOURCES).is_boolean());
    // exportedAsSinks
    assert(config.contains(EXPORTED_AS_SINKS));
    assert(config.at(EXPORTED_AS_SINKS).is_boolean());
    // blackList
    assert(config.contains(BLACKLIST));
    assert(config.at(BLACKLIST).is_array());
    for (auto const& item : config.at(BLACKLIST).items()) {
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
        if (item.value().contains(SIZE)) {
            assert(item.value().at(SIZE).is_number_integer());
            assert(item.value().at(SIZE) >= 0);
        }
    }
    // formatString
    assert(config.contains(FORMAT_STRING));
    assert(config.at(FORMAT_STRING).is_object());
    for (auto const& item : config.at(FORMAT_STRING).items()) {
        assert(item.value().is_number_integer());
        assert(item.value() >= 0);
    }
}

void wasmati::checkUnreachableCode(json& config,
                                   std::list<Vulnerability>& vulns) {
    for (auto func : Query::functions(Query::ALL_NODES)) {
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
    }
}

void wasmati::checkBufferOverflow(json& config,
                                  std::list<Vulnerability>& vulns) {
    checkBoBuffsStatic(config, vulns);
    checkBoScanfLoops(config, vulns);
}

void wasmati::checkBoBuffsStatic(json& config,
                                 std::list<Vulnerability>& vulns) {
    json boConfig = config.at(BUFFER_OVERFLOW);
    std::set<std::string> funcSink;

    for (auto const& kv : boConfig.items()) {
        funcSink.insert(kv.key());
    }

    for (auto func : Query::functions()) {
        auto buffers = checkBufferSizes(func);
        if (buffers.empty()) {
            continue;
        }

        NodeSet queryCalls = Query::instructions({func}, [&](Node* node) {
            return node->instType() == ExprType::Call &&
                   funcSink.count(node->label()) == 1;
        });

        for (auto call : queryCalls) {
            json callSink = boConfig.at(call->label());
            if (!callSink.contains(BUFFER) ||
                !callSink.at(BUFFER).is_number()) {
                assert(false);
            }
            // buffer
            int indexBuffer;
            callSink.at(BUFFER).get_to(indexBuffer);
            Node* bufferArg =
                call->getOutEdge(indexBuffer, EdgeType::AST)->dest();

            // limit
            int indexLimit = -1;
            if (callSink.contains(SIZE)) {
                callSink.at(SIZE).get_to(indexLimit);
            } else {
                vulns.emplace_back(VulnType::BufferOverflow, func->name(),
                                   call->label(), "");
            }
            Node* limitArg =
                call->getOutEdge(indexLimit, EdgeType::AST)->dest();

            // lookup buffer
            int bufferPosition = 0;
            NodeSet queryBuff = Query::BFSincludes(
                {bufferArg},
                [&](Node* node) {
                    if (node->instType() == ExprType::Binary &&
                        node->opcode() == Opcode::I32Add) {
                        auto inEdges = node->inEdges(EdgeType::PDG);
                        EdgeSet set(inEdges.begin(), inEdges.end());
                        auto containsSP = Query::containsEdge(set, [](Edge* e) {
                            return e->pdgType() == PDGType::Global &&
                                   e->label() == "$g0";
                        });
                        auto containsConst =
                            Query::containsEdge(set, [&](Edge* e) {
                                return e->pdgType() == PDGType::Const &&
                                       (bufferPosition = e->value().u32) > 0;
                            });
                        if (inEdges.size() >= 2 && containsSP &&
                            containsConst) {
                            return true;
                        }
                    }
                    if (node->instType() == ExprType::Binary &&
                        node->opcode() == Opcode::I32Sub) {
                        auto inEdges = node->inEdges(EdgeType::PDG);
                        EdgeSet set(inEdges.begin(), inEdges.end());
                        auto containsSP = Query::containsEdge(set, [](Edge* e) {
                            return e->pdgType() == PDGType::Global &&
                                   e->label() == "$g0";
                        });
                        auto containsConst =
                            Query::containsEdge(set, [&](Edge* e) {
                                return e->pdgType() == PDGType::Const;
                            });
                        if (inEdges.size() >= 2 && containsSP &&
                            containsConst) {
                            return true;
                        }
                    }
                    return false;
                },
                Query::PDG_EDGES, 1, true);
            if (queryBuff.size() != 1) {
                continue;
            }

            // lookup size
            int sizeToWrite = 0;
            auto outEdges = limitArg->outEdges(EdgeType::PDG);
            if (outEdges.size() == 1 &&
                (*outEdges.begin())->pdgType() == PDGType::Const) {
                sizeToWrite = (*outEdges.begin())->value().u32;
            } else {
                continue;
            }

            int sizeAlloc = buffers.rbegin()->first + buffers.rbegin()->second;
            if (bufferPosition < 32 &&
                sizeToWrite > sizeAlloc - bufferPosition) {
                std::string desc =
                    "buffer @+" + std::to_string(bufferPosition) + " is " +
                    std::to_string(sizeAlloc - bufferPosition) +
                    " and is expecting " + std::to_string(sizeToWrite);
                vulns.emplace_back(VulnType::BufferOverflow, func->name(),
                                   call->label(), desc);
            } else if (bufferPosition >= 32 &&
                       sizeToWrite > buffers[bufferPosition]) {
                std::string desc =
                    "buffer @+" + std::to_string(bufferPosition) + " is " +
                    std::to_string(buffers[bufferPosition]) +
                    " and is expecting " + std::to_string(sizeToWrite);
                vulns.emplace_back(VulnType::BufferOverflow, func->name(),
                                   call->label(), desc);
            }
        }
    }
}

void wasmati::checkBoScanfLoops(json& config, std::list<Vulnerability>& vulns) {
    for (auto func : Query::functions()) {
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

void wasmati::checkFormatString(json& config, std::list<Vulnerability>& vulns) {
    json fsConfig = config.at(FORMAT_STRING);

    for (auto func : Query::functions()) {
        if (fsConfig.contains(func->name())) {
            continue;
        }
        auto query = Query::instructions({func}, [&](Node* node) {
            if (node->instType() == ExprType::Call &&
                fsConfig.contains(node->label())) {
                auto child = node->getChild(fsConfig.at(node->label()));
                auto childEdges = child->outEdges(EdgeType::PDG);
                return !Query::containsEdge(childEdges, [](Edge* e) {
                    return e->pdgType() == PDGType::Const;
                });
            }
            return false;
        });

        // write vulns
        for (auto call : query) {
            vulns.emplace_back(VulnType::FormatStrings, func->name(),
                               call->label());
        }
    }
}

std::map<int, int> wasmati::checkBufferSizes(Node* func) {
    std::map<int, int> buffers;
    EdgeSet queryEdges;
    Index totalSizeAllocated;
    EdgeCondition edgeCond = [](Edge* e) {
        return (e->pdgType() == PDGType::Const &&
                static_cast<int>(e->value().u32) > 0) ||
               (e->pdgType() == PDGType::Global &&
                e->label().compare("$g0") == 0);
    };
    auto allocQuery = Query::instructions({func}, [&](Node* node) {
        if (node->instType() == ExprType::Binary &&
            node->opcode() == Opcode::I32Sub) {
            auto inEdges = node->inEdges(EdgeType::PDG);
            auto queryEdges = Query::filterEdges(inEdges, edgeCond);
            return queryEdges.size() == 2;
        }
        return false;
    });

    if (allocQuery.size() == 1) {
        auto inEdges = (*allocQuery.begin())->inEdges(EdgeType::PDG);
        queryEdges = Query::filterEdges(inEdges, edgeCond);
    }

    if (allocQuery.size() != 1 || queryEdges.size() != 2) {
        return buffers;
    }

    for (Edge* e : queryEdges) {
        if (e->pdgType() == PDGType::Const) {
            totalSizeAllocated = e->value().u32;
        }
    }

    std::set<int> buffs;
    buffs.insert(0);
    auto buffQuery = Query::instructions({func}, [&](Node* node) {
        if (node->instType() == ExprType::Binary &&
            node->opcode() == Opcode::I32Add) {
            auto inEdges = node->inEdges(EdgeType::PDG);
            auto queryEdges = Query::filterEdges(inEdges, [&](Edge* e) {
                return (e->pdgType() == PDGType::Const &&
                        static_cast<int>(e->value().u32) > 0 &&
                        static_cast<int>(e->value().u32) <
                            totalSizeAllocated) ||
                       (e->pdgType() == PDGType::Global &&
                        e->label().compare("$g0") == 0);
            });
            auto querySub = Query::BFS(
                {node}, [&](Node* n) { return n == *allocQuery.begin(); },
                Query::PDG_EDGES, 1, true);
            return queryEdges.size() == 2 && querySub.size() == 1;
        }
        return false;
    });

    for (Node* node : buffQuery) {
        for (Edge* e : node->inEdges(EdgeType::PDG)) {
            if (e->pdgType() == PDGType::Const) {
                buffs.insert(e->value().u32);
            }
        }
    }
    for (auto it = buffs.begin(); it != buffs.end(); ++it) {
        int size = 0;
        if (std::next(it) != buffs.end()) {
            size = (*std::next(it)) - *it;
        } else {
            size = totalSizeAllocated - *it;
        }
        buffers[*it] = size;
    }
    return buffers;
}

void wasmati::checkTainted(json& config, std::list<Vulnerability>& vulns) {
    std::set<std::string> sources = config.at(SOURCES);
    std::set<std::string> sinks = config.at(SINKS);

    if (config.at(IMPORT_AS_SOURCES)) {
        auto funcs = Query::map<std::string>(
            Query::functions([](Node* node) { return node->isImport(); }),
            [](Node* node) { return node->name(); });
        sources.insert(funcs.begin(), funcs.end());
    }

    if (config.at(EXPORTED_AS_SINKS)) {
        auto funcs = Query::map<std::string>(
            Query::functions([](Node* node) { return node->isExport(); }),
            [](Node* node) { return node->name(); });
        sinks.insert(funcs.begin(), funcs.end());
    }

    auto tainted = config.at(TAINTED);

    for (auto func : Query::functions()) {
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
                        vulns.emplace_back(VulnType::Tainted, func->name(), "",
                                           desc.str());
                        return true;
                    }
                    return false;
                });
            }
            return false;
        });
    }
}

void wasmati::checkIntegerOverflow(json& config) {}

void wasmati::checkUseAfterFree(json& config) {}
