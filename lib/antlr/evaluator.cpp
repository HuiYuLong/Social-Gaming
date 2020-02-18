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

#include "antlr4-runtime.h"
#include "SimpleBooleanParser.h"
#include "SimpleBooleanLexer.h"
#include "SimpleBooleanBaseVisitor.h"

using namespace antlrcpp;
using namespace antlr4;
using namespace std;

// class  SimpleBooleanEvaluator : public SimpleBooleanBaseVisitor {
// public:

//   virtual antlrcpp::Any visitParse(SimpleBooleanParser::ParseContext *ctx) override {
//     return visitChildren(ctx);
//   }

//   virtual antlrcpp::Any visitBinaryExpression(SimpleBooleanParser::BinaryExpressionContext *ctx) override {
//     return visitChildren(ctx);
//   }

//   virtual antlrcpp::Any visitDecimalExpression(SimpleBooleanParser::DecimalExpressionContext *ctx) override {
//     return visitChildren(ctx);
//   }

//   virtual antlrcpp::Any visitBoolExpression(SimpleBooleanParser::BoolExpressionContext *ctx) override {
//     return visitChildren(ctx);
//   }

//   virtual antlrcpp::Any visitIdentifierExpression(SimpleBooleanParser::IdentifierExpressionContext *ctx) override {
//     return visitChildren(ctx);
//   }

//   virtual antlrcpp::Any visitNotExpression(SimpleBooleanParser::NotExpressionContext *ctx) override {
//     return visitChildren(ctx);
//   }

//   virtual antlrcpp::Any visitParenExpression(SimpleBooleanParser::ParenExpressionContext *ctx) override {
//     return visitChildren(ctx);
//   }

//   virtual antlrcpp::Any visitComparatorExpression(SimpleBooleanParser::ComparatorExpressionContext *ctx) override {
//     return visitChildren(ctx);
//   }

//   virtual antlrcpp::Any visitComparator(SimpleBooleanParser::ComparatorContext *ctx) override {
//     return visitChildren(ctx);
//   }

//   virtual antlrcpp::Any visitBinary(SimpleBooleanParser::BinaryContext *ctx) override {
//     return visitChildren(ctx);
//   }

//   virtual antlrcpp::Any visitBoolean(SimpleBooleanParser::BooleanContext *ctx) override {
//     return visitChildren(ctx);
//   }
// };


int main(int argc, const char* argv[]) {
    if (argc != 2) {
        std::cout << "Enter input file name" << std::endl;
        return 1;
    }
	ifstream testfile (argv[1]);
	if (testfile.is_open()) {
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
		testfile.close();
	}

  return 0;
}
