/* Copyright (c) 2012-2017 The ANTLR Project. All rights reserved.
 * Use of this file is governed by the BSD 3-clause license that
 * can be found in the LICENSE.txt file in the project root.
 */

//
//  main.cpp
//  antlr4-cpp-demo
//
//  Created by Mike Lischke on 13.03.16.
//

#include <iostream>
#include <exception>

#include "antlr4-runtime.h"
#include "SimpleBooleanParser.h"
#include "SimpleBooleanLexer.h"
#include "SimpleBooleanBaseVisitor.h"

#include "variables.h"

using namespace antlrcpp;
using namespace antlr4;
using namespace std;

class  SimpleBooleanEvaluator : public SimpleBooleanBaseVisitor {
  const Variable& toplevel;
public:

  SimpleBooleanEvaluator(const Variable& toplevel): toplevel(toplevel) {}

  virtual Any visitParse(SimpleBooleanParser::ParseContext *ctx) override {
    return visitChildren(ctx->expression());
  }

  virtual Any visitBinaryExpression(SimpleBooleanParser::BinaryExpressionContext *ctx) override {
	if (ctx->op->AND() != nullptr)
	{
		return visitChildren(ctx->left) && visitChildren(ctx->right);
	}
	else if (ctx->op->OR() != nullptr)
	{
		return visitChildren(ctx->left) || visitChildren(ctx->right);
	}
	else
	{
		throw std::invalid_argument("Invalid binary operator");
	}
  }

  virtual Any visitDecimalExpression(SimpleBooleanParser::DecimalExpressionContext *ctx) override {
    return std::stoi(ctx->getText());
  }

  virtual Any visitBoolExpression(SimpleBooleanParser::BoolExpressionContext *ctx) override {
    return (ctx->getText() == "true" ? true : false);
  }

  virtual Any visitIdentifierExpression(SimpleBooleanParser::IdentifierExpressionContext *ctx) override {
	SuperGetter getter(ctx->getText());
    return boost::apply_visitor(getter, toplevel);
  }

  virtual Any visitNotExpression(SimpleBooleanParser::NotExpressionContext *ctx) override {
    return !visitChildren(ctx);
  }

  virtual Any visitParenExpression(SimpleBooleanParser::ParenExpressionContext *ctx) override {
    return visitChildren(ctx->expression());
  }

  virtual Any visitComparatorExpression(SimpleBooleanParser::ComparatorExpressionContext *ctx) override {
    if (ctx->op->EQ() != nullptr) {
      return boost::apply_visitor(Equal(), visitChildren(ctx->left), visitChildren(ctx->right));
    }
    else if (ctx->op->LE() != nullptr) {
      return boost::apply_visitor(LessOrEqual(), visitChildren(ctx->left), visitChildren(ctx->right));
    }
    else if (ctx->op->GE() != nullptr) {
      return boost::apply_visitor(GreaterOrEqual(), visitChildren(ctx->left), visitChildren(ctx->right));
    }
    else if (ctx->op->LT() != nullptr) {
      return boost::apply_visitor(Less(), visitChildren(ctx->left), visitChildren(ctx->right));
    }
    else if (ctx->op->GT() != nullptr) {
      return boost::apply_visitor(Greater(), visitChildren(ctx->left), visitChildren(ctx->right));
    }
    throw std::invalid_argument("not implemented: comparator operator " + ctx->op->getText());
  }

};

// Temporarily
#include <nlohmann/json.hpp>

// Again, temporarily here
Variable buildVariables(const nlohmann::json& json)
{
    if (json.is_boolean()) {
        //std::cout << json << std::endl;
        return (Variable) (bool) json;
    }
    else if (json.is_number()) {
        //std::cout << json << std::endl;
        return (Variable) (int) json;
    }
    else if (json.is_string()) {
        //std::cout << json << std::endl;
        return (Variable) (std::string) json;
    }
    else if (json.is_object()) {
        Map map;
        for(const auto&[key, value]: json.items()) {
            //std::cout << key << std::endl;
            map[key] = buildVariables(value);
        }
        return (Variable) map;
    }
    else if (json.is_array()) {
        List list;
        for(const auto&[key, value]: json.items()) {
            //std::cout << key << std::endl;
            list.push_back(buildVariables(value));
        }
        return (Variable) list;
    }
    else {
        std::cout << "Invalid JSON variable type" << std::endl;
        std::terminate();
    }
}

int main(int argc, const char* argv[]) {
    if (argc != 3) {
        std::cout << "Enter input file name and a test json game configuration file" << std::endl;
        return 1;
    }
	ifstream testfile (argv[1]);
	if (testfile.fail())
	{
		std::cout << "cannot open the input test file" << std::endl;
		return 0;
	}

	ANTLRInputStream input(testfile);
	SimpleBooleanLexer lexer(&input);
	CommonTokenStream tokens(&lexer);

	tokens.fill();
	for (auto token : tokens.getTokens()) {
		std::cout << token->toString() << std::endl;
	}

	SimpleBooleanParser parser(&tokens);
	tree::ParseTree* tree = parser.parse();
	std::cout << tree->toStringTree(&parser) << std::endl << std::endl;

	// TEST
	std::ifstream config{argv[2]};
	if (config.fail())
	{
		std::cout << "cannot open the game congiguration file" << std::endl;
		return 0;
	}
	nlohmann::json gamespec = nlohmann::json::parse(config);

	std::vector<Variable> player_names;
	for (const std::string& name : {"a", "b", "c"})
		player_names.push_back(name);

	Map map;
	// Put "setup" variables into "configuration" submap
	map["configuration"] = Map();
	Map& configuration = boost::get<Map>(map["configuration"]);
	for(const auto&[key, value]: gamespec["configuration"]["setup"].items()) {
		configuration[key] = buildVariables(value);
	}
	// Put variables into the top-level map
	for(const auto&[key, value]: gamespec["variables"].items()) {
		map[key] = buildVariables(value);
	}
	// Put constants into the top-level map
	for(const auto&[key, value]: gamespec["constants"].items()) {
		map[key] = buildVariables(value);
	}
	// Add players
	map["players"] = List();
	List& players = boost::get<List>(map["players"]);
	for(const Variable& name: player_names) {
		Map player = boost::get<Map>(buildVariables(gamespec["per-player"]));
		player["name"] = name;
		players.push_back(player);
	}
	// Who cares
	map["audience"] = List();


	SimpleBooleanEvaluator evaluator(map);
	std::cout << "Result: " << (bool) evaluator.visitParse(parser.parse()) << std::endl;
	

  return 0;
}
