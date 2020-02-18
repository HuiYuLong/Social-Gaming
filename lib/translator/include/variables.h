#include <boost/variant.hpp>
#include <boost/tokenizer.hpp>
#include <iostream>
#include <numeric>

// using String = std::string;
// using Integer = long;
// using Boolean = bool;
// using Key = std::string;

using Variable = boost::make_recursive_variant<
    bool,   
    int,
    std::string,
    std::vector<boost::recursive_variant_>,
    std::unordered_map<std::string, boost::recursive_variant_ >
>::type;

using List = std::vector<Variable>;
using Map = std::unordered_map<std::string, Variable>;

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
class SuperGetter : public boost::static_visitor<Variable&>
{
    Variable returned;
    static boost::char_separator<char> dot;
    tokenizer tokens;
    tokenizer::iterator it;
public:
    SuperGetter(const tokenizer& query):
        tokens(query), it(tokens.begin()) {}

    SuperGetter(const std::string& untokenizer_query):
        tokens(untokenizer_query, dot), it(tokens.begin()) {}

    Variable& operator()(bool& boolean)
    {
        return (Variable&) boolean;
    }

    Variable& operator()(int& integer)
    {
        if (it == tokens.end())
            return (Variable&) integer;
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

    Variable& operator()(std::string& string)
    {
        return (Variable&) string;
    }

    Variable& operator()(List& list)
    {
        if (it == tokens.end())
            return (Variable&) list;
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

    Variable& operator()(Map& map)
    {
        return boost::apply_visitor(*this, map[*(it++)]);
    }
};

//Variable SuperGetter::returned = List();

boost::char_separator<char> SuperGetter::dot(".");

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
};

// query is a string like 'configuration.Rounds.upfrom(1)', tokenized by the '.'
// TODO: input validation is easy to add
// Variable& superGetter(Variable& variable, const std::vector<std::string>& query, size_t current_token = 0u)
// {
//     if (current_token == query.size())
//         return variable;
//     switch
// }