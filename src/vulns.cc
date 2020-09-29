#include "vulns.h"
using namespace wasmati;

void wasmati::checkVulnerabilities(Graph* graph,
                                   json& config,
                                   std::list<Vulnerability>& vulns) {
    Query::setGraph(graph);
    checkUnreachableCode(config, vulns);
    checkBufferOverflow(config, vulns);
    // checkIntegerOverflow();
    // checkUseAfterFree();
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
    const std::string KEY_BO = "bufferOverflow";
    if (!config.contains(KEY_BO)) {
        return;
    }

    json boConfig = config.at(KEY_BO);
    std::set<std::string> funcSink;

    for (auto const& kv : boConfig.items()) {
        funcSink.insert(kv.key());
    }

    for (auto func : Query::functions(Query::ALL_NODES)) {
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
            if (!callSink.contains("buffer") ||
                !callSink.at("buffer").is_number()) {
                assert(false);
            }
            // buffer
            int indexBuffer;
            callSink.at("buffer").get_to(indexBuffer);
            Node* bufferArg =
                call->getOutEdge(indexBuffer, EdgeType::AST)->dest();

            // limit
            int indexLimit = -1;
            if (callSink.contains("size")) {
                callSink.at("size").get_to(indexLimit);
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
                outEdges[0]->pdgType() == PDGType::Const) {
                sizeToWrite = outEdges[0]->value().u32;
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

void wasmati::checkIntegerOverflow(json& config) {}

void wasmati::checkUseAfterFree(json& config) {}

std::map<int, int> wasmati::checkBufferSizes(Node* func) {
    std::map<int, int> buffers;
    EdgeSet queryEdges;
    Index totalSizeAllocated;
    auto allocQuery = Query::instructions({func}, [&](Node* node) {
        if (node->instType() == ExprType::Binary &&
            node->opcode() == Opcode::I32Sub) {
            auto inEdges = node->inEdges(EdgeType::PDG);
            EdgeSet edgeSet = EdgeSet(inEdges.begin(), inEdges.end());
            queryEdges = Query::filterEdges(edgeSet, [](Edge* e) {
                return e->pdgType() == PDGType::Const ||
                       (e->pdgType() == PDGType::Global &&
                        e->label().compare("$g0") == 0);
            });
            return queryEdges.size() == 2;
        }
        return false;
    });

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
            EdgeSet edgeSet = EdgeSet(inEdges.begin(), inEdges.end());
            auto queryEdges = Query::filterEdges(edgeSet, [&](Edge* e) {
                return (e->pdgType() == PDGType::Const && e->value().u32 > 0 &&
                        e->value().u32 < totalSizeAllocated) ||
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
