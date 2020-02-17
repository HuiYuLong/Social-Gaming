
// Generated from SimpleBoolean.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"
#include "SimpleBooleanListener.h"


/**
 * This class provides an empty implementation of SimpleBooleanListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  SimpleBooleanBaseListener : public SimpleBooleanListener {
public:

  virtual void enterParse(SimpleBooleanParser::ParseContext * /*ctx*/) override { }
  virtual void exitParse(SimpleBooleanParser::ParseContext * /*ctx*/) override { }

  virtual void enterBinaryExpression(SimpleBooleanParser::BinaryExpressionContext * /*ctx*/) override { }
  virtual void exitBinaryExpression(SimpleBooleanParser::BinaryExpressionContext * /*ctx*/) override { }

  virtual void enterDecimalExpression(SimpleBooleanParser::DecimalExpressionContext * /*ctx*/) override { }
  virtual void exitDecimalExpression(SimpleBooleanParser::DecimalExpressionContext * /*ctx*/) override { }

  virtual void enterBoolExpression(SimpleBooleanParser::BoolExpressionContext * /*ctx*/) override { }
  virtual void exitBoolExpression(SimpleBooleanParser::BoolExpressionContext * /*ctx*/) override { }

  virtual void enterIdentifierExpression(SimpleBooleanParser::IdentifierExpressionContext * /*ctx*/) override { }
  virtual void exitIdentifierExpression(SimpleBooleanParser::IdentifierExpressionContext * /*ctx*/) override { }

  virtual void enterNotExpression(SimpleBooleanParser::NotExpressionContext * /*ctx*/) override { }
  virtual void exitNotExpression(SimpleBooleanParser::NotExpressionContext * /*ctx*/) override { }

  virtual void enterParenExpression(SimpleBooleanParser::ParenExpressionContext * /*ctx*/) override { }
  virtual void exitParenExpression(SimpleBooleanParser::ParenExpressionContext * /*ctx*/) override { }

  virtual void enterComparatorExpression(SimpleBooleanParser::ComparatorExpressionContext * /*ctx*/) override { }
  virtual void exitComparatorExpression(SimpleBooleanParser::ComparatorExpressionContext * /*ctx*/) override { }

  virtual void enterComparator(SimpleBooleanParser::ComparatorContext * /*ctx*/) override { }
  virtual void exitComparator(SimpleBooleanParser::ComparatorContext * /*ctx*/) override { }

  virtual void enterBinary(SimpleBooleanParser::BinaryContext * /*ctx*/) override { }
  virtual void exitBinary(SimpleBooleanParser::BinaryContext * /*ctx*/) override { }

  virtual void enterBoolean(SimpleBooleanParser::BooleanContext * /*ctx*/) override { }
  virtual void exitBoolean(SimpleBooleanParser::BooleanContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

