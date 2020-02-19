#include <boost/variant.hpp>
#include <boost/tokenizer.hpp>
#include <boost/function.hpp>
#include <iostream>
#include <numeric>
#include <unordered_map>

// using String = std::string;
// using Integer = long;
// using Boolean = bool;
// using Key = std::string;

using Variable = boost::make_recursive_variant<
    bool,   
    int,
    std::string,
    std::vector<boost::recursive_variant_>,
    std::unordered_map<std::string, boost::recursive_variant_ >,
    boost::recursive_variant_*
>::type;

using List = std::vector<Variable>;
using Map = std::unordered_map<std::string, Variable>;
using Pointer = Variable*;

class Equal
    : public boost::static_visitor<bool>
{
public:

    template <typename T, typename U>
    bool operator()( const T &, const U & ) const
    {
        return false; // cannot compare different types
    }

    template <typename T>
    bool operator()( const T & lhs, const T & rhs ) const
    {
        return lhs == rhs;
    }

};

class NotEqual
    : public boost::static_visitor<bool>
{
public:

    template <typename T, typename U>
    bool operator()( const T &, const U & ) const
    {
        return false; // cannot compare different types
    }

    template <typename T>
    bool operator()( const T & lhs, const T & rhs ) const
    {
        return lhs != rhs;
    }

};

class Less
    : public boost::static_visitor<bool>
{
public:

    template <typename T, typename U>
    bool operator()( const T &, const U & ) const
    {
        return false; // cannot compare different types
    }

    bool operator()(int lhs, int rhs ) const
    {
        return lhs < rhs;
    }

};

class LessOrEqual
    : public boost::static_visitor<bool>
{
public:

    template <typename T, typename U>
    bool operator()( const T &, const U & ) const
    {
        return false; // cannot compare different types
    }

    bool operator()(int lhs, int rhs ) const
    {
        return lhs <= rhs;
    }

};

class Greater
    : public boost::static_visitor<bool>
{
public:

    template <typename T, typename U>
    bool operator()( const T &, const U & ) const
    {
        return false; // cannot compare different types
    }

    bool operator()(int lhs, int rhs ) const
    {
        return lhs > rhs;
    }

};

class GreaterOrEqual
    : public boost::static_visitor<bool>
{
public:

    template <typename T, typename U>
    bool operator()( const T &, const U & ) const
    {
        return false; // cannot compare different types
    }

    bool operator()(int lhs, int rhs ) const
    {
        return lhs >= rhs;
    }

};


using tokenizer = boost::tokenizer<boost::char_separator<char> >;

// Processes the query of the type 'configuration.Rounds.upfrom(1)'
// query is tokenized by the '.'
class SuperGetter : public boost::static_visitor<const Variable&>
{
    thread_local static Variable returned;
    static boost::char_separator<char> dot;
    tokenizer tokens;
    tokenizer::iterator it;
public:
    SuperGetter(const tokenizer& query):
        tokens(query), it(tokens.begin()) {}

    SuperGetter(const std::string& untokenizer_query):
        tokens(untokenizer_query, dot), it(tokens.begin()) {}

    const Variable& operator()(const bool& boolean)
    {
        return boolean;
    }

    const Variable& operator()(const int& integer)
    {
        if (it == tokens.end())
            return integer;
        auto next = it;
        ++next;
        if (next == tokens.end())
        {
            const std::string& current_query = *it;
            if(current_query.compare(0, 6, "upfrom") == 0)
            {
                size_t opening_bracket = current_query.rfind('(');
                size_t closing_bracket = current_query.rfind(')');
                int from = std::stoi(current_query.substr(opening_bracket + 1, closing_bracket - opening_bracket - 1));
                returned = List(integer - from + 1, 0);
                List& upfrom = boost::get<List>(returned);
                //upfrom.reserve(integer - from + 1);
                std::iota(upfrom.begin(), upfrom.end(), from);
                return returned;
            }
            else
            {
                std::cout << "Unrecognized integer attribute" << std::endl;
                std::terminate();
            }
        }
        else
        {
            std::cout << "Ill-formed variable access" << std::endl;
            std::terminate();
        }
    }

    const Variable& operator()(const std::string& string)
    {
        return string;
    }

    const Variable& operator()(const List& list)
    {
        if (it == tokens.end())
            return list;
        const auto& current_query = *it;
        if(current_query.compare(0, 4, "size") == 0)
        {
            // Ugly, but works
            // Might separate read-only and writable getters later
            returned = (int) list.size();
            return returned;
        }
        else if(current_query.compare(0, 8, "contains") == 0)
        {
            // TODO
            return (Variable&) list;
        }
        else if(current_query.compare(0, 7, "collect") == 0)
        {
            // TODO
            return (Variable&) list;
        }
        else if(current_query.compare(0, 8, "elements") == 0)
        {
            // TODO
            return (Variable&) list;
        }
        else
        {
            std::cout << "Unrecognized list attribute" << std::endl;
            std::terminate();
        }
    }

    const Variable& operator()(const Map& map)
    {
        return boost::apply_visitor(*this, map.at(*(it++)));
    }

    const Variable& operator()(const Pointer& ptr)
    {
        return boost::apply_visitor(*this, *ptr);
    }
};

thread_local Variable SuperGetter::returned;

boost::char_separator<char> SuperGetter::dot(".");

// Attempt #2

struct GetterResult
{
    Variable& result;
    bool needs_to_be_saved;
};

class Getter
{
    thread_local static Variable returned;
    static boost::char_separator<char> dot;
    tokenizer tokens;
    tokenizer::iterator it;
public:
    Getter(const std::string& untokenizer_query);
    GetterResult get_from(Variable& toplevel);
    GetterResult processBoolean(Variable* boolean);
    GetterResult processInteger(Variable* integer);
    GetterResult processString(Variable* string);
    GetterResult processList(Variable* varlist);
    GetterResult processMap(Variable* varmap);
    GetterResult processPointer(Variable* varptr);
};

using Callmap = std::vector<boost::function<GetterResult(Getter*, Variable*)>>;

Callmap callmap = {
    [](Getter* getter, Variable* boolean) { return getter->processBoolean(boolean); },
    [](Getter* getter, Variable* integer) { return getter->processInteger(integer); },
    [](Getter* getter, Variable* string) { return getter->processString(string); },
    [](Getter* getter, Variable* list) { return getter->processList(list); },
    [](Getter* getter, Variable* map) { return getter->processMap(map); },
    [](Getter* getter, Variable* pointer) { return getter->processPointer(pointer); }
};

Getter::Getter(const std::string& untokenizer_query):
    tokens(untokenizer_query, dot), it(tokens.begin()) {}

GetterResult Getter::processBoolean(Variable* boolean)
{
    return {*boolean, false};
}

GetterResult Getter::processInteger(Variable* integer)
{
    if (it == tokens.end())
        return {*integer, false};
    auto next = it;
    ++next;
    if (next == tokens.end())
    {
        const std::string& current_query = *it;
        if(current_query.compare(0, 6, "upfrom") == 0)
        {
            size_t opening_bracket = current_query.rfind('(');
            size_t closing_bracket = current_query.rfind(')');
            int from = std::stoi(current_query.substr(opening_bracket + 1, closing_bracket - opening_bracket - 1));
            returned = List(boost::get<int>(*integer) - from + 1, 0);
            List& upfrom = boost::get<List>(returned);
            //upfrom.reserve(integer - from + 1);
            std::iota(upfrom.begin(), upfrom.end(), from);
            return {returned, true};
        }
        else
        {
            std::cout << "Unrecognized integer attribute" << std::endl;
            std::terminate();
        }
    }
    else
    {
        std::cout << "Ill-formed variable access" << std::endl;
        std::terminate();
    }
}

GetterResult Getter::processString(Variable* string)
{
    return {*string, false};
}

GetterResult Getter::processList(Variable* varlist)
{
    if (it == tokens.end())
        return {*varlist, false};
    const auto& current_query = *it;
    List& list = boost::get<List>(*varlist);
    if(current_query.compare(0, 4, "size") == 0)
    {
        // Ugly, but works
        // Might separate read-only and writable getters later
        returned = (int) list.size();
        return {returned, true};
    }
    else if(current_query.compare(0, 8, "contains") == 0)
    {
        // TODO
        return {*varlist, true};
    }
    else if(current_query.compare(0, 7, "collect") == 0)
    {
        // TODO
        return {*varlist, true};
    }
    else if(current_query.compare(0, 8, "elements") == 0)
    {
        // TODO
        return {*varlist, true};
    }
    else
    {
        std::cout << "Unrecognized list attribute" << std::endl;
        std::terminate();
    }
}

GetterResult Getter::processMap(Variable* varmap)
{
    if (it == tokens.end()) {
        return {*varmap, false};
    }
    Map& map = boost::get<Map>(*varmap);
    Variable& next = map.at(*(it++));
    return callmap[next.which()](this, &next);
}

GetterResult Getter::processPointer(Variable* varptr)
{
    Pointer& ptr = boost::get<Pointer>(*varptr);
    return callmap[ptr->which()](this, ptr);
}

GetterResult Getter::get_from(Variable& toplevel)
{
    return callmap[toplevel.which()](this, &toplevel);
}

thread_local Variable Getter::returned;

boost::char_separator<char> Getter::dot(".");

// using Callmap = std::vector<boost::function<int(int)>>;

// Callmap callmap = {
//     [](int boolean) { return boolean; },
//     [](int integer) { return integer; },
//     [](int string) { return string; },
//     [](int list) { return list; },
//     [](int map) { return map; },
//     [](int pointer) { return pointer; }
// };

class PrintTheThing : public boost::static_visitor<>
{
    size_t current_offset = 0u;
public:
    //PrintTheThing(): current_offset(0u) {}

    void print_offset() const { for(size_t i = 0; i < current_offset; i++) std::cout << ' '; }

    void operator()(bool boolean) const
    {
        print_offset();
        std::cout << (boolean ? "true" : "false") << std::endl;
    }

    void operator()(int integer) const
    {
        print_offset();
        std::cout << integer << std::endl;
    }

    void operator()(const std::string& string) const
    {
        print_offset();
        std::cout << string << std::endl;
    }

    void operator()(const List& list)
    {
        current_offset++;
        for(const Variable& var : list) {
            boost::apply_visitor(*this, var);
        }
        current_offset--;
    }

    void operator()(const Map& map)
    {
        
        for(const auto&[key, var] : map) {
            print_offset();
            std::cout << key << std::endl;
            current_offset++;
            boost::apply_visitor(*this, var);
            current_offset--;
        }
    }

    void operator()(const Pointer& ptr)
    {
        boost::apply_visitor(*this, *ptr);
    }
};

class StringConverter : public boost::static_visitor<std::string>
{
public:

    std::string operator()(bool boolean) const
    {
        return (boolean ? "true" : "false");
    }

    std::string operator()(int integer) const
    {
        return std::to_string(integer);
    }

    std::string operator()(const std::string& string) const
    {
        return string;
    }

    std::string operator()(const List& list) const
    {
        std::ostringstream out;
        out << "[";
        for(const Variable& var : list) {
            out << boost::apply_visitor(*this, var) << ", ";
        }
        out.seekp(out.tellp() - std::ostringstream::streampos(2));
        out << "]";
        return out.str();
    }

    std::string operator()(const Map& map) const
    {
        std::ostringstream out;
        out << "{";
        for(const auto&[key, var] : map) {
            out << key << ": " << boost::apply_visitor(*this, var) << ", ";
        }
        out.seekp(out.tellp() - std::ostringstream::streampos(2));
        out << "}";
        return out.str();
    }

    std::string operator()(const Pointer& ptr) const
    {
        return boost::apply_visitor(*this, *ptr);
    }
};

// query is a string like 'configuration.Rounds.upfrom(1)', tokenized by the '.'
// TODO: input validation is easy to add
// Variable& superGetter(Variable& variable, const std::vector<std::string>& query, size_t current_token = 0u)
// {
//     if (current_token == query.size())
//         return variable;
//     switch
// }