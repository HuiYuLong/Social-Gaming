#include "variables.h"

bool operator==(const Query& q1, const Query& q2)
{
    return q1.query == q2.query;
}

thread_local Variable Getter::returned;

boost::char_separator<char> Getter::dot(".");

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

Getter::Getter(const std::string& untokenizer_query, Variable& toplevel):
    tokens(untokenizer_query, dot), it(tokens.begin()), toplevel(toplevel) {}

GetterResult Getter::processBoolean(Variable& boolean)
{
    return {boolean, false};
}

GetterResult Getter::processInteger(Variable& integer)
{
    // Return the integer if it is the last token in the query
    if (it == tokens.end()) {
        return {integer, false};
    }
    auto next = it;
    ++next;
    // If the integer has a method call, process it
    if (next == tokens.end())
    {
        const std::string& current_query = *it;
        if(current_query.compare(0, 6, "upfrom") == 0)
        {
            size_t opening_bracket = current_query.rfind('(');
            size_t closing_bracket = current_query.rfind(')');
            int from = std::stoi(current_query.substr(opening_bracket + 1, closing_bracket - opening_bracket - 1));
            returned = List(boost::get<int>(integer) - from + 1, 0);
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

GetterResult Getter::processString(Variable& string)
{
    return {string, false};
}

GetterResult Getter::processList(Variable& varlist)
{
    if (it == tokens.end())
        return {varlist, false};
    const auto& current_query = *it;
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
        return {varlist, true};
    }
    else if(current_query.compare(0, 7, "collect") == 0)
    {
        // TODO
        return {varlist, true};
    }
    else if(current_query.compare(0, 8, "elements") == 0)
    { 
        List tmp;
        auto next = it;
        ++next;
        const auto& next_query = *next;
        // get the key value (ie. name)
        std::size_t found = next_query.rfind('(');
        if(found==std::string::npos) {
            std::transform(list.begin(), list.end(), std::back_inserter(tmp),
                [&next_query] (const Variable& item) {
                    return boost::get<Map>(item).at(next_query);
                });
        }

        returned = tmp;
        return {returned, true};
    }
    else
    {
        std::cout << "Unrecognized list attribute" << std::endl;
        std::terminate();
    }
}

GetterResult Getter::processMap(Variable& varmap)
{
    if (it == tokens.end()) {
        return {varmap, false};
    }
    Map& map = boost::get<Map>(varmap);
    Variable& next = map.at(*(it++));
    return callmap[next.which()](this, next);
}

GetterResult Getter::processPointer(Variable& varptr)
{
    Pointer& ptr = boost::get<Pointer>(varptr);
    return callmap[ptr->which()](this, *ptr);
}

GetterResult Getter::processQuery(Variable& query)
{
    const std::string& query_string = boost::get<Query>(query).query;
    Getter subgetter(query_string, toplevel);
    return subgetter.get();
}

GetterResult Getter::get()
{
    return callmap[toplevel.which()](this, toplevel);
}