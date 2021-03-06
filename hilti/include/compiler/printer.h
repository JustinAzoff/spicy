// Copyright (c) 2020 by the Zeek Project. See LICENSE for details.

#pragma once

#include <iostream>

#include <hilti/base/util.h>
#include <hilti/compiler/detail/visitors.h>

namespace hilti::printer {

class Stream {
public:
    Stream(std::ostream& s, bool _compact) : _stream(s), _compact(_compact), _nl(_compact ? ' ' : '\n') {}

    void beginLine() { _stream << std::string(_indent * 4, ' '); }
    void endLine() { _stream << (_compact ? ' ' : '\n'); }
    void emptyLine() {
        if ( _wrote_nl )
            return;

        _stream << (_compact ? ' ' : '\n');
        _wrote_nl = true;
    }

    char newline() const { return _nl; }

    const ID& currentScope() const { return _scopes.back(); }
    void pushScope(ID id) { _scopes.push_back(std::move(id)); }
    void popScope() { _scopes.pop_back(); }

    bool isCompact() { return _compact; }

    bool isExpandSubsequentType() const { return _expand_subsequent_type; }
    void setExpandSubsequentType(bool expand) { _expand_subsequent_type = expand; }

    bool isFirstInBlock() const { return _first_in_block; }
    bool isLastInBlock() const { return _last_in_block; }
    void setPositionInBlock(bool first, bool last) {
        _first_in_block = first;
        _last_in_block = last;
    }

    auto indent() const { return _indent; }
    void incrementIndent() { ++_indent; }
    void decrementIndent() {
        --_indent;
        _first_in_block = _last_in_block = false;
    }

    template<typename T, IF_DERIVED_FROM(T, trait::isNode)>
    Stream& operator<<(const T& t) {
        if constexpr ( std::is_base_of<trait::isType, T>::value ) {
            if ( auto id = Type(t).typeID() )
                _stream << *id;
        }
        else
            hilti::detail::printAST(t, *this);

        return *this;
    }

    template<typename T, IF_NOT_DERIVED_FROM(T, trait::isNode)>
    Stream& operator<<(const T& t) {
        _wrote_nl = false;
        _stream << t;
        _expand_subsequent_type = false;
        return *this;
    }

    // Output lists.
    template<typename T>
    Stream& operator<<(std::pair<T, const char*> p) {
        bool first = true;
        for ( auto& i : p.first ) {
            if ( ! first )
                _stream << p.second;

            (*this) << i;
            first = false;
        }

        return *this;
    }

private:
    std::ostream& _stream;
    bool _compact;
    char _nl;
    int _indent = 0;
    bool _wrote_nl = false;
    bool _first_in_block = false;
    bool _last_in_block = false;
    bool _expand_subsequent_type = false;
    std::vector<ID> _scopes = {""};
};

} // namespace hilti::printer
