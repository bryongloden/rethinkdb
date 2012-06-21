#ifndef RDB_PROTOCOL_QUERY_LANGUAGE_HPP__
#define RDB_PROTOCOL_QUERY_LANGUAGE_HPP__

#include <list>
#include <deque>

#include "utils.hpp"
#include <boost/variant.hpp>

#include "rdb_protocol/query_language.pb.h"

//TODO maybe we can merge well definedness and type checking. */

/* Make sure that the protocol buffers we receive are a well defined type. That
 * is they specifiy which type they are and have the correct optional fields
 * filled in (and none of the others). */

bool is_well_defined(const VarTermTuple &);

bool is_well_defined(const Term &);

bool is_well_defined(const Builtin &);

bool is_well_defined(const Reduction &);

bool is_well_defined(const Mapping &);

bool is_well_defined(const Predicate &);

bool is_well_defined(const View &);

bool is_well_defined(const Builtin &);

bool is_well_defined(const WriteQuery &);

namespace query_language {
struct error_t;
struct primitive_t;

typedef boost::variant<error_t, primitive_t> type_t;

struct error_t { 
    error_t()
        : desc("Unknown error.")
    { }

    explicit error_t(const std::string &_desc)
        : desc(_desc)
    { }

    std::string desc;

    bool operator==(const error_t &e) const {
        return desc == e.desc;
    }
};

struct primitive_t {
    enum primitive_type_t {
        ERROR,
        JSON,
        STREAM
    } value;

    explicit primitive_t(primitive_type_t _value)
        : value(_value)
    { }

    bool operator==(const primitive_t &p) const {
        return value == p.value;
    }
};


typedef std::list<type_t> function_t;

class variable_type_scope_t {
public:
    void put_in_scope(std::string name, type_t type) {
        rassert(!scopes.empty());
        scopes.front()[name] = type;
    }

    type_t get_type(std::string name) {
        for (scopes_t::iterator it  = scopes.begin();
                                it != scopes.end();
                                ++it) {
            std::map<std::string, type_t>::iterator jt = it->find(name);
            if (jt != it->end()) {
                return jt->second;
            }
        }
        
        return error_t(strprintf("Symbol %s is not in scope\n", name.c_str()));
    }

    void push() {
        scopes.push_front(std::map<std::string, type_t>());
    }

    void pop() {
        scopes.pop_front();
    }
private:
    typedef std::deque<std::map<std::string, type_t> > scopes_t;
    scopes_t scopes;
};

/* get_type functions assume that the contained value is well defined. */
type_t get_type(const Term &t, variable_type_scope_t *scope);

function_t get_type(const Builtin &b, variable_type_scope_t *scope);
} //namespace query_language

#endif
