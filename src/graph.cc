#include "src/graph.h"

namespace wasmati {

int Node::idCount = 0;

std::string Edge::toString() {
	return std::to_string(_src->getId()) + " -> " + std::to_string(_dest->getId()) + " [color=black]\n";
}

std::string ASTEdge::toString() {
	return std::to_string(_src->getId()) + " -> " + std::to_string(_dest->getId()) + " [color=green]\n";
}

std::string CFGEdge::toString() {
	return std::to_string(_src->getId()) + " -> " + std::to_string(_dest->getId()) + " [color=red]\n";
}

std::string PDGEdge::toString() {
	return std::to_string(_src->getId()) + " -> " + std::to_string(_dest->getId()) + " [color=blue]\n";
}

std::string Node::edgesToString() {
	std::string s;
	
	for (Edge* e : _edges) {
		s  += e->toString();
	}
	return s;
}

std::string Module::toString() {
	std::string s = this->edgesToString();
	s += std::to_string(_id) + " [label=<<TABLE><TR><TD>module</TD></TR>";
	s += "<TR><TD>";
	s += "name = " + _name + "</TD></TR></TABLE>> ];";
	return s;
}


std::string Function::toString() {
	std::string s = this->edgesToString();
	s += std::to_string(_id) + " [label=<<TABLE><TR><TD>function</TD></TR>";
	s += "<TR><TD>";
	s += "name = " + _name + "</TD></TR></TABLE>> ];";
	return s;
}


void Graph::writePuts(wabt::Stream* stream, const char* s) {
	size_t len = strlen(s);
	stream->WriteData(s, len);
	stream->WriteChar('\n');
}

void Graph::writeString(wabt::Stream* stream, const std::string& str) {
	writePuts(stream, str.c_str());
}

void Graph::generateAST() {
	Module* m = new Module("Modulo");
	Function* f1 = new Function("f1");
	Function* f2 = new Function("f2");
	Edge* e1 = new ASTEdge(m, f1);
	Edge* e2 = new ASTEdge(m, f2);
	Edge* e3 = new CFGEdge(f1, f2);
	m->addEdge(e1);
	m->addEdge(e2);
	f1->addEdge(e3);
	insertNode(m);
	insertNode(f1);
	insertNode(f2);
}

wabt::Result Graph::writeGraph(wabt::Stream* stream) {
	writePuts(stream, "digraph G {");
	writePuts(stream, "graph [rankdir=TD];");
	writePuts(stream, "node [shape=none];");
	
	for (auto const& node : _nodes) {
		writeString(stream, node->toString());
	}
	writePuts(stream, "}");
	return wabt::Result::Ok;
}

}  // namespace wasmati

