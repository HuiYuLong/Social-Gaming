#include "variables.h"
#include <boost/regex.hpp>
#include <boost/convert.hpp>
#include <boost/convert/strtol.hpp>
// #include <charconv> //cannot include for some reason

bool operator==(const Query& q1, const Query& q2)
{
    return q1.query == q2.query;
}

thread_local Variable Getter::returned;

std::regex Condition::equality_regex("\\s*(\\S+)\\s*==\\s*(\\S+)\\s*");
std::regex Condition::decimal_regex("\\d+");
//std::regex Condition::variable_regex("(\\w+(\\(\\w+\\)))?\\w+(\\(\\w+\\)))?");

using Callmap = std::vector<boost::function<GetterResult(Getter*, Variable&)>>;

Callmap callmap = {
    [](Getter* getter, Variable& boolean) { return getter->processBoolean(boolean); },
    [](Getter* getter, Variable& integer) { return getter->processInteger(integer); },
    [](Getter* getter, Variable& string) { return getter->processString(string); },
    [](Getter* getter, Variable& list) { return getter->processList(list); },
    [](Getter* getter, Variable& map) { return getter->processMap(map); },
    [](Getter* getter, Variable& pointer) { return getter->processPointer(pointer); },
    [](Getter* getter, Variable& query) { return getter->processQuery(query); }
};

Getter::Getter(const Query& query, Variable& toplevel):
    iterator(query), toplevel(toplevel) {}

GetterResult Getter::processBoolean(Variable& boolean)
{
    return {boolean, false};
}

GetterResult Getter::processInteger(Variable& integer)
{
    // Return the integer if it is the last token in the query
    if (!iterator.hasNext()) {
        return {integer, false};
    }
    const std::string_view current_query = iterator.getNext();
    if(current_query.compare(0, 6, "upfrom") == 0)
    {
        size_t opening_bracket = current_query.rfind('(');
        size_t closing_bracket = current_query.rfind(')');
        std::string_view argument = current_query.substr(opening_bracket + 1, closing_bracket - opening_bracket - 1);
        // int upfrom;
        // if(auto [p, ec] = std::from_chars(argument.data(), argument.data()+argument.size(), upfrom); ec != std::errc()) {
        //     std::cout << "Error in upfrom(int): invalid argument" << std::endl;
        //     std::terminate();
        // }
        auto value = boost::convert<int>(argument, boost::cnv::strtol());
        if (!value.has_value())
        {
            std::cout << "Error in upfrom(int): invalid argument" << std::endl;
            std::terminate();
        }
        int upfrom = value.get();
        returned = List(boost::get<int>(integer) - upfrom + 1, 0);
        List& upfrom_list = boost::get<List>(returned);
        std::iota(upfrom_list.begin(), upfrom_list.end(), upfrom);
        return {returned, true};
    }
    else
    {
        std::cout << "Unrecognized integer attribute" << std::endl;
        std::terminate();
    }
}

GetterResult Getter::processString(Variable& string)
{
    if(iterator.hasNext()) {
        std::cout << "Error: strings have no attributes" << std::endl;
        std::terminate();
    }
    return {string, false};
}

GetterResult Getter::processList(Variable& varlist)
{
    if (!iterator.hasNext())
        return {varlist, false};
    const auto current_query = iterator.getNext();
    List& list = boost::get<List>(varlist);
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
        size_t opening_bracket = current_query.rfind('(');
        size_t closing_bracket = current_query.rfind(')');
        std::string_view contains_what = current_query.substr(opening_bracket + 1, closing_bracket - opening_bracket - 1);

        Getter subgetter(Query{contains_what}, toplevel);
        // Might need to save it if it's temporary
        const Variable& contained_variable = subgetter.get().result;

        Equal equal(toplevel);
        bool contains = std::any_of(list.begin(), list.end(), [&contained_variable, equal](const Variable& element) {
            return equal(contained_variable, element);
        });
        returned = contains;

        return {returned, true};
    }
    else if(current_query.compare(0, 7, "collect") == 0)
    {
        size_t opening_bracket = 7u;
        size_t closing_bracket = current_query.size() - 1u;
        if (current_query.size() < 12u /*smallest possible: collect(x,y)*/
            || current_query[opening_bracket] != '('
            || current_query[closing_bracket] != ')') {
                std::cout << "Invalid query: collect method call is ill-formed" << std::endl;
                std::terminate();
            }
        size_t argument_separator = current_query.find(',', opening_bracket);
        if(argument_separator == std::string::npos) {
            std::cout << "Imvalid query: collect method requires two arguments" << std::endl;
            std::terminate();
        }
        size_t second_argument_start = argument_separator + 1u;
        while (current_query[second_argument_start] == ' ') { ++second_argument_start; }
        const auto element_name = current_query.substr(opening_bracket + 1u, argument_separator - opening_bracket - 1u);
        const auto condition_str = current_query.substr(second_argument_start, closing_bracket - second_argument_start);
        std::string element_name_as_string(element_name);
        List collected_list;
        collected_list.reserve(list.size());
        Condition condition(condition_str);
        Map& toplevel_map = boost::get<Map>(toplevel);
        for (Variable& element : list) {
            toplevel_map[element_name_as_string] = &element;
            if (condition.evaluate(toplevel)) {
                collected_list.push_back(element);
            }
        }
        if(!iterator.hasNext()) {
            returned = std::move(collected_list);
            return {returned, true};
        }
        Variable temp = std::move(collected_list);
        GetterResult result = processList(temp);
        return {result.result, true};   // ensure that needs_to_be_saved is true
    }
    else if(current_query == "elements")
    { 
        List elements_list;
        elements_list.reserve(list.size());
        if(!iterator.hasNext()) {
            std::cout << "Invalid query: \"elements\" must be followed by an attribute name" << std::endl;
            std::terminate();
        }
        const auto attribute = iterator.getNext();
        std::transform(list.begin(), list.end(), std::back_inserter(elements_list), [&attribute] (const Variable& item) {
            return boost::get<Map>(item).at(std::string{attribute});
        });
        if(!iterator.hasNext()) {
            returned = std::move(elements_list);
            return {returned, true};
        }
        Variable temp = std::move(elements_list);
        GetterResult result = processList(temp);
        return {result.result, true};   // ensure that needs_to_be_saved is true
    }
    else
    {
        std::cout << "Unrecognized list attribute" << std::endl;
        std::terminate();
    }
}

GetterResult Getter::processMap(Variable& varmap)
{
    if (!iterator.hasNext()) {
        return {varmap, false};
    }
    Map& map = boost::get<Map>(varmap);
    Variable& next = map.at(std::string{iterator.getNext()});
    return callmap[next.which()](this, next);
}

GetterResult Getter::processPointer(Variable& varptr)
{
    Pointer& ptr = boost::get<Pointer>(varptr);
    return callmap[ptr->which()](this, *ptr);
}

GetterResult Getter::processQuery(Variable& varquery)
{
    const Query& query = boost::get<Query>(varquery);
    Getter subgetter(query, toplevel);
    return subgetter.get();
}

GetterResult Getter::get()
{
    return callmap[toplevel.which()](this, toplevel);
}

void Getter::setQuery(Query query)
{
    iterator = QueryTokensIterator(query);
}


Equal::Equal(Variable& toplevel): toplevel(toplevel) {}

template <typename T, typename U>
bool Equal::operator()(const T& lhs, const U& rhs) const
{
    return false; // cannot compare different types
}

template <typename U>
bool Equal::operator()(const Query& query, const U& rhs) const
{
    Getter getter(query, toplevel);
    U& value = boost::get<U>(getter.get().result);
    return value == rhs;
}

template <typename T>
bool Equal::operator()(const T& lhs, const Query& query) const
{
    return this->operator()(query, lhs);
}

bool Equal::operator()(const Query& query1, const Query& query2) const
{
    Getter getter1(query1, toplevel);
    Variable lhs = getter1.get().result;    // save the first one
    Getter getter2(query2, toplevel);
    Variable& rhs = getter2.get().result;   // take a reference to the second one
    return boost::apply_visitor(*this, lhs, rhs);
}

template <typename T>
bool Equal::operator()( const T & lhs, const T & rhs ) const
{
    return lhs == rhs;
}

Variable Condition::getOperand(const std::string& str)
{
    if (std::regex_match(str, decimal_regex)) {
        return std::stoi(str);
    }
    else {
        // Assume it's a variable name
        return Query(str);
    }
}

void Condition::init(std::string_view condition)
{
    bool negated;
    if (condition.at(0) == '!') {
        negated = true;
        condition = condition.substr(1);
    }
    std::match_results<std::string_view::const_iterator> match;
    if (std::regex_match(condition.cbegin(), condition.cend(), match, equality_regex)) {
        // Interpret as a comparison of two variables
        clause = [first=getOperand(match.str(1)), second=getOperand(match.str(2)), negated] (Variable& toplevel) {
            Equal equal(toplevel);
            return negated ^ boost::apply_visitor(equal, first, second);
        };
    }
    else {
        // Interpret as a boolean variable
        clause = [query = Query(condition), negated] (Variable& toplevel) {
            Getter getter(query, toplevel);
            return negated ^ boost::get<bool>(getter.get().result);
        };
    }
}

Condition::Condition(std::string_view condition)
{
    init(condition);
}

Condition::Condition(const nlohmann::json& condition)
{
    if (condition.is_boolean())
    {
        if (condition)
            clause = [](Variable&) { return true; };
        else
            clause = [](Variable&) { return false; };
    }
    else
    {
        // Assume condition is given as a string
        std::string condition_str = condition;
        init(condition_str);
    }
}

// evaluate condition
bool Condition::evaluate(Variable& toplevel) { return clause(toplevel); }