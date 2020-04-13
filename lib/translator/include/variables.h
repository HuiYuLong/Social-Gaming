#include <boost/variant.hpp>
#include <boost/function.hpp>
#include <boost/algorithm/string/join.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <numeric>
#include <unordered_map>
#include <algorithm>
#include <regex>

// using String = std::string;
// using Integer = long;
// using Boolean = bool;
// using Key = std::string;

// Contains a variable name or expression (like configuration.Rounds.upfrom(1))
struct Query
{
    std::string query;

    Query(std::string_view str): query(str) {}

    Query(std::string&& str): query(str) {}
};

bool operator==(const Query& q1, const Query& q2);

std::ostream& operator<<(std::ostream& out, const Query& query);

using Variable = boost::make_recursive_variant<
    bool,   
    int,
    std::string,
    std::vector<boost::recursive_variant_>,
    std::unordered_map<std::string, boost::recursive_variant_ >,
    boost::recursive_variant_*,
    Query
>::type;

using List = std::vector<Variable>;
using Map = std::unordered_map<std::string, Variable>;
using Pointer = Variable*;

Pointer getReference(Variable& variable);

struct GetterError : public std::runtime_error
{
    GetterError(const char* message): std::runtime_error(message) { }
};

class QueryTokensIterator
{
    std::string query_string;
    size_t last_pos;

public:
    QueryTokensIterator(const Query& query): query_string(query.query), last_pos(0u) {}

    QueryTokensIterator(const std::string&& query): query_string(query), last_pos(0u) {}

    std::string_view produceToken(size_t separator_pos)
    {
        if (separator_pos <= last_pos) {
            throw GetterError{"Tokenizer error: empty token\n"};
        }
        std::string_view token{query_string.data() + last_pos, separator_pos - last_pos};
        last_pos = separator_pos + 1;
        return token;
    }

    std::string_view getNext()
    {
        for(size_t pos = last_pos; pos < query_string.size(); ++pos)
        {
            if (query_string[pos] == '.') {
                return produceToken(pos);
            }
            if (query_string[pos] == '(') {
                do {
                    pos++;
                    if(pos == query_string.size()) {
                        throw GetterError{"Tokenizer error: function call doesn't have a closing bracket\n"};
                    }
                } while (query_string[pos] != ')');
                pos++;
                if (pos == query_string.size() || query_string[pos] == '.') {
                    return produceToken(pos);
                }
                else {
                    throw GetterError{"Tokenizer error: function call not followed by a separator\n"};
                }
            }
        }
        return produceToken(query_string.size());
    }

    bool hasNext() { return last_pos < query_string.size(); }
};

// I tried to use a visitor to access the variables by their names
// but I couldn't return references to variants from it

// class ConstantGetter : public boost::static_visitor<const Variable&>
// {
//     thread_local static Variable returned;
//     static boost::char_separator<char> dot;
//     tokenizer tokens;
//     tokenizer::iterator it;
// public:
//     ConstantGetter(const tokenizer& query):
//         tokens(query), it(tokens.begin()) {}

//     ConstantGetter(const std::string& untokenizer_query):
//         tokens(untokenizer_query, dot), it(tokens.begin()) {}

//     const Variable& operator()(const bool& boolean)
//     {
//         return boolean;
//     }

//     const Variable& operator()(const int& integer)
//     {
//         if (it == tokens.end())
//             return integer;
//         auto next = it;
//         ++next;
//         if (next == tokens.end())
//         {
//             const std::string& current_query = *it;
//             if(current_query.compare(0, 6, "upfrom") == 0)
//             {
//                 size_t opening_bracket = current_query.rfind('(');
//                 size_t closing_bracket = current_query.rfind(')');
//                 int from = std::stoi(current_query.substr(opening_bracket + 1, closing_bracket - opening_bracket - 1));
//                 returned = List(integer - from + 1, 0);
//                 List& upfrom = boost::get<List>(returned);
//                 //upfrom.reserve(integer - from + 1);
//                 std::iota(upfrom.begin(), upfrom.end(), from);
//                 return returned;
//             }
//             else
//             {
//                 std::cout << "Unrecognized integer attribute" << std::endl;
//                 std::terminate();
//             }
//         }
//         else
//         {
//             std::cout << "Ill-formed variable access" << std::endl;
//             std::terminate();
//         }
//     }

//     const Variable& operator()(const std::string& string)
//     {
//         return string;
//     }

//     const Variable& operator()(const List& list)
//     {
//         if (it == tokens.end())
//             return list;
//         const auto& current_query = *it;
//         if(current_query.compare(0, 4, "size") == 0)
//         {
//             // Ugly, but works
//             // Might separate read-only and writable getters later
//             returned = (int) list.size();
//             return returned;
//         }
//         else if(current_query.compare(0, 8, "contains") == 0)
//         {
//             // TODO
//             return (Variable&) list;
//         }
//         else if(current_query.compare(0, 7, "collect") == 0)
//         {
//             // TODO
//             return (Variable&) list;
//         }
//         else if(current_query.compare(0, 8, "elements") == 0)
//         {
//             // TODO
//             return (Variable&) list;
//         }
//         else
//         {
//             std::cout << "Unrecognized list attribute" << std::endl;
//             std::terminate();
//         }
//     }

//     const Variable& operator()(const Map& map)
//     {
//         return boost::apply_visitor(*this, map.at(*(it++)));
//     }

//     const Variable& operator()(const Pointer& ptr)
//     {
//         return boost::apply_visitor(*this, *ptr);
//     }
// };

// thread_local Variable ConstantGetter::returned;

// boost::char_separator<char> ConstantGetter::dot(".");

// Attempt #2

// The reference to the requested variable and a flag
// that indicates whether the variable needs to be copied or moved
// before making any other variable accesses
struct GetterResult
{
    Variable& result;
    bool needs_to_be_saved;
};

// Processes the query of the type 'configuration.Rounds.upfrom(1)'
// query is tokenized by the '.'
class Getter
{
    thread_local static Variable returned;
    Variable& toplevel;
    QueryTokensIterator iterator;
    bool create_if_not_exists;
public:
    Getter(const Query& untokenizer_query, Variable& toplevel);
    void setQuery(Query query);
    GetterResult get(bool create_if_not_exists = false);
    GetterResult processBoolean(Variable& boolean);
    GetterResult processInteger(Variable& integer);
    GetterResult processString(Variable& string);
    GetterResult processList(Variable& varlist);
    GetterResult processMap(Variable& varmap);
    GetterResult processPointer(Variable& varptr);
    GetterResult processQuery(Variable& varquery);
};

// The job of the Condition class is to prepare a function that corresponds
// to a logical expression inside the game specification (like players.wins == 0)
// During the game, you just need to call the evaluate method with the current variable tree
// that automatically accesses all variables involved in the logical expression
// and returns its result as a boolean variable
class Condition 
{
    // The clause will take in a top-level variable map and return a boolean value
    // Indicating whether the variables satisfy the clause
    std::function<bool(Variable&)> clause;
    static std::regex equality_regex;
    static std::regex decimal_regex;
  
    Variable getOperand(const std::string& str);

    void init(std::string_view condition);
public:
    Condition(std::string_view condition);

    Condition(const nlohmann::json& condition);

    Condition(const Variable&, const Variable&);

    // evaluate condition
    bool evaluate(Variable& toplevel);
};


// Prints the variable to the standard output
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

    void operator()(const Query& query)
    {
        print_offset();
        std::cout << "{" << query.query << "}" << std::endl;
    }
};

// Converts arbitrary variable into a string
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
        if(list.size() == 0u) {
            return "[]";
        }
        std::ostringstream out;
        out << "[" <<  boost::apply_visitor(*this, *list.begin());
        for(auto it = ++list.begin(); it != list.end(); ++it) {
            out << ", " << boost::apply_visitor(*this, *it) ;
        }
        out << "]";
        return out.str();
    }

    std::string operator()(const Map& map) const
    {
        if (map.size() == 0u) {
            return "{}";
        }
        std::ostringstream out;
        out << "{" << map.begin()->first << ": " << boost::apply_visitor(*this, map.begin()->second);
        for(auto it = ++map.begin(); it != map.end(); ++it) {
            out <<  ", " << it->first << ": " << boost::apply_visitor(*this, it->second);
        }
        out << "}";
        return out.str();
    }

    std::string operator()(const Pointer& ptr) const
    {
        return boost::apply_visitor(*this, *ptr);
    }

    std::string operator()(const Query& query) const
    {
        return "{" + query.query + "}";
    }
};

//------------------------------------------Comparison operators-----------------------------------------

// Tests any two variables for equality
// Capable of automatically getting the values of the queries
class Equal
    : public boost::static_visitor<bool>
{
    Variable& toplevel;
public:

    Equal(Variable& toplevel);

    template <typename T, typename U>
    bool operator()(const T& lhs, const U& rhs) const;

    template <typename U>
    bool operator()(const Query& query, const U& rhs) const;

    template <typename T>
    bool operator()(const T& lhs, const Query& query) const;

    bool operator()(const Query& query1, const Query& query2) const;

    template <typename T>
    bool operator()( const T & lhs, const T & rhs ) const;

};

// Not needed yet, but may be implemented

// class NotEqual
//     : public boost::static_visitor<bool>
// {
// public:

//     template <typename T, typename U>
//     bool operator()( const T &, const U & ) const
//     {
//         return false; // cannot compare different types
//     }

//     template <typename T>
//     bool operator()( const T & lhs, const T & rhs ) const
//     {
//         return lhs != rhs;
//     }

// };

// class Less
//     : public boost::static_visitor<bool>
// {
// public:

//     template <typename T, typename U>
//     bool operator()( const T &, const U & ) const
//     {
//         return false; // cannot compare different types
//     }

//     bool operator()(int lhs, int rhs ) const
//     {
//         return lhs < rhs;
//     }

// };

// class LessOrEqual
//     : public boost::static_visitor<bool>
// {
// public:

//     template <typename T, typename U>
//     bool operator()( const T &, const U & ) const
//     {
//         return false; // cannot compare different types
//     }

//     bool operator()(int lhs, int rhs ) const
//     {
//         return lhs <= rhs;
//     }

// };

// class Greater
//     : public boost::static_visitor<bool>
// {
// public:

//     template <typename T, typename U>
//     bool operator()( const T &, const U & ) const
//     {
//         return false; // cannot compare different types
//     }

//     bool operator()(int lhs, int rhs ) const
//     {
//         return lhs > rhs;
//     }

// };

// class GreaterOrEqual
//     : public boost::static_visitor<bool>
// {
// public:

//     template <typename T, typename U>
//     bool operator()( const T &, const U & ) const
//     {
//         return false; // cannot compare different types
//     }

//     bool operator()(int lhs, int rhs ) const
//     {
//         return lhs >= rhs;
//     }

// };