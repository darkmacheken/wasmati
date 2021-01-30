#ifndef WASMATI_SymbolTable_H
#define WASMATI_SymbolTable_H

namespace wasmati {

template <typename Symbol>
class SymbolTable {
private:
    typedef typename std::map<std::string, Symbol> context_type;
    // typedef typename context_type::const_iterator context_iterator;

    /** the context level */
    int _level;

    /** this is the current context */
    context_type* _current;

    /** stores a context path from the root to the current (last) context */
    std::vector<context_type*> _contexts;

public:
    SymbolTable() : _level(0) {
        _current = new context_type;
        _contexts.push_back(_current);
    }

    /**
     * Destroy the symbol table and all symbol pointers it contains.
     */
    virtual ~SymbolTable() {
        while (_level > 0)
            pop();
        destroyCurrent();
        _contexts.clear();
    }

private:
    /**
     * Destroy all symbol pointers in the current context.
     * Delete the current context and set the pointer to null.
     */
    void destroyCurrent() {
        _current->clear();
        delete _current;
        _current = nullptr;
    }

public:
    /**
     * Create a new context and make it current.
     */
    void push() {
        _level++;
        _current = new context_type;
        _contexts.push_back(_current);
    }

    /**
     * Destroy the current context: the previous context becomes the current
     * one. If the first context is reached, no operation is performed.
     */
    void pop() {
        if (_level == 0)
            return;
        destroyCurrent();
        _contexts.pop_back();
        _current = _contexts.back();
        _level--;
    }

    /**
     * Define a new identifier in the local (current) context.
     *
     * @param name the symbol's name.
     * @param symbol the symbol.
     * @return
     *   <tt>true</tt> if new identifier (may be defined in an upper
     *   context); <tt>false</tt> if identifier already exists in the
     *   current context.
     */
    bool insert(const std::string& name, Symbol symbol) {
        auto it = _current->find(name);
        if (it == _current->end()) {
            (*_current)[name] = symbol;
            return true;
        }
        return false;
    }

    /**
     * Replace the data corresponding to a symbol in the current context.
     *
     * @param name the symbol's name.
     * @param symbol the symbol.
     * @return
     *   <tt>true</tt> if the symbol exists; <tt>false</tt> if the
     *   symbol does not exist in any of the contexts.
     */
    bool replace_local(const std::string& name, Symbol symbol) {
        auto it = _current->find(name);
        if (it != _current->end()) {
            (*_current)[name] = symbol;
            return true;
        }
        return false;
    }

    /**
     * Replace the data corresponding to a symbol (look for the symbol in all
     * available contexts, starting with the innermost one).
     *
     * @param name the symbol's name.
     * @param symbol the symbol.
     * @return
     *   <tt>true</tt> if the symbol exists; <tt>false</tt> if the
     *   symbol does not exist in any of the contexts.
     */
    bool replace(const std::string& name, Symbol symbol) {
        for (size_t ix = _contexts.size(); ix > 0; ix--) {
            context_type& ctx = *_contexts[ix - 1];
            auto it = ctx.find(name);
            if (it != ctx.end()) {
                // FIXME: BUG: should free previous symbol
                ctx[name] = symbol;
                return true;
            }
        }
        return false;
    }

    /**
     * Search for a symbol in the local (current) context.
     *
     * @param name the symbol's name.
     * @param symbol the symbol.
     * @return
     *   <tt>true</tt> if the symbol exists; <tt>false</tt> if the
     *   symbol does not exist in the current context.
     */
    Symbol find_local(const std::string& name) {
        auto it = _current->find(name);
        if (it != _current->end())
            return it->second;  // symbol data
        return nullptr;
    }

    /**
     * Search for a symbol in the avaible contexts, starting with the first
     * one and proceeding until reaching the outermost context.
     *
     * @param name the symbol's name.
     * @param from how many contexts up from the current one (zero).
     * @return
     *    <tt>nullptr</tt> if the symbol cannot be found in any of the
     *    contexts; or the symbol and corresponding attributes.
     */
    Symbol find(const std::string& name, size_t from = 0) const {
        if (from >= _contexts.size())
            return nullptr;
        for (size_t ix = _contexts.size() - from; ix > 0; ix--) {
            context_type& ctx = *_contexts[ix - 1];
            auto it = ctx.find(name);
            if (it != ctx.end())
                return it->second;  // symbol data
        }
        return nullptr;
    }
};
}  // namespace wasmati
#endif
