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

using Callmap = std::vector<boost::function<GetterResult(Getter*, Variable&)>>;

Callmap callmap = {
    [](Getter* getter, Variable& boolean) { 
        std::cout << "1*************" << std::endl;
        return getter->processBoolean(boolean); },
    [](Getter* getter, Variable& integer) { 
        std::cout << "2*************" << std::endl;
        return getter->processInteger(integer); },
    [](Getter* getter, Variable& string) { 
        std::cout << "3*************" << std::endl;
        return getter->processString(string); },
    [](Getter* getter, Variable& list) { 
        std::cout << "4*************" << std::endl;
        return getter->processList(list); },
    [](Getter* getter, Variable& map) { 
        std::cout << "5*************" << std::endl;
        return getter->processMap(map); },
    [](Getter* getter, Variable& pointer) { 
        std::cout << "6*************" << std::endl;
        return getter->processPointer(pointer); },
    [](Getter* getter, Variable& query) { 
        std::cout << "7*************" << std::endl;
        return getter->processQuery(query); }
};

Getter::Getter(const std::string& untokenized_query, Variable& toplevel):
    iterator(untokenized_query), toplevel(toplevel) {}

GetterResult Getter::processBoolean(Variable& boolean)
{
    std::cout << "processing the boollllran" <<std::endl;
    return {boolean, false};
}

GetterResult Getter::processInteger(Variable& integer)
{
    std::cout << "processing the interrrrrger" <<std::endl;
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
    std::cout << "Processing the stringgggg" << std::endl;
    if(iterator.hasNext()) {
        std::cout << "Error: strings have no attributes" << std::endl;
        std::terminate();
    }
    return {string, false};
}

GetterResult Getter::processList(Variable& varlist)
{
    std::cout << "processing the listt *****************" << std::endl;
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
        std::cout << "in contains()" << std::endl;
        // TODO
        size_t opening_bracket = current_query.rfind('(');
        size_t closing_bracket = current_query.rfind(')');
        std::string_view contains_what = current_query.substr(opening_bracket + 1, closing_bracket - opening_bracket - 1);

        Getter subgetter(std::string{contains_what}, toplevel);
        // Might need to save it if it's temporary
        const Variable& contained_variable = subgetter.get().result;

        Equal equal(toplevel);
        bool contains = std::any_of(list.begin(), list.end(), [contained_variable, equal](const Variable& element) {
            return equal(contained_variable, element);
        });
        returned = contains;

        return {returned, true};
    }
    else if(current_query.compare(0, 7, "collect") == 0)
    {
        // TODO
        std::cout << "NO22222 *****************" << std::endl;
        return {varlist, true};
    }
    else if(current_query == "elements")
    { 
        std::cout << "in elements" << std::endl;
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
    std::cout << "Processing the mapppppppp" << std::endl;
    if (!iterator.hasNext()) {
        return {varmap, false};
    }
    Map& map = boost::get<Map>(varmap);
    Variable& next = map.at(std::string{iterator.getNext()});
    return callmap[next.which()](this, next);
}

GetterResult Getter::processPointer(Variable& varptr)
{
    std::cout << "processing the pointerrrr" << std::endl;
    Pointer& ptr = boost::get<Pointer>(varptr);
    return callmap[ptr->which()](this, *ptr);
}

GetterResult Getter::processQuery(Variable& query)
{
    std::cout << "Processing the queryyyyyyyyyyy" << std::endl;
    const std::string& query_string = boost::get<Query>(query).query;
    Getter subgetter(query_string, toplevel);
    return subgetter.get();
}

GetterResult Getter::get()
{
    std::cout << "get whattt" << std::endl;
    return callmap[toplevel.which()](this, toplevel);
}