#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <cmath>
#include <map>
#include <ostream>
#include <stdexcept>
#include <vector>

/** Context used to save the parsed expressions. This context is
 * passed along to the wasmati::Interpreter class and fill during parsing via bison
 * actions. */
class Context {
public:
    /// type of the variable storage
    typedef std::map<std::string, double> variablemap_type;

    /// variable storage. maps variable string to doubles
    variablemap_type variables;

    /// array of unassigned expressions found by the parser. these are then
    /// outputted to the user.
    //std::vector<CalcNode*> expressions;

    /// free the saved expression trees
    ~Context() { //clearExpressions(); 
    }

    /// free all saved expression trees
    //void clearExpressions() {
    //    for (unsigned int i = 0; i < expressions.size(); ++i) {
    //        delete expressions[i];
    //    }
    //    expressions.clear();
    //}

    /// check if the given variable name exists in the storage
    bool existsVariable(const std::string& varname) const {
        return variables.find(varname) != variables.end();
    }

    /// return the given variable from the storage. throws an exception if it
    /// does not exist.
    double getVariable(const std::string& varname) const {
        variablemap_type::const_iterator vi = variables.find(varname);
        if (vi == variables.end())
            throw(std::runtime_error("Unknown variable."));
        else
            return vi->second;
    }
};

#endif  // EXPRESSION_H
