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
#include "SimpleBooleanBaseListener.h"

using namespace antlrcpp;
using namespace antlr4;
using namespace std;

// class SimpleBooleanListener : public SimpleBooleanBaseListener {
// public:
//   void enterKey(ParserRuleContext *ctx) override {
// 	std::cout << "a" << std::endl;
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
        SimpleBooleanParser::ParseContext* tree = parser.parse();

        // tree::ParseTree *tree = parser.key();
        // SimpleBooleanBaseListener listener;
        // tree::ParseTreeWalker::DEFAULT->walk(&listener, tree);
		testfile.close();
	}
//   ANTLRInputStream input(u8"🍴 = 🍐 + \"😎\";(((x * π))) * µ + ∰; a + (x * (y ? 0 : 1) + z);");
//   SimpleBooleanLexer lexer(&input);
//   CommonTokenStream tokens(&lexer);

//   tokens.fill();
//   for (auto token : tokens.getTokens()) {
//     std::cout << token->toString() << std::endl;
//   }

//   SimpleBooleanParser parser(&tokens);
//   tree::ParseTree* tree = parser.main();

//   std::cout << tree->toStringTree(&parser) << std::endl << std::endl;

  return 0;
}
