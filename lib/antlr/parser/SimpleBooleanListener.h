
// Generated from SimpleBoolean.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"
#include "SimpleBooleanParser.h"


/**
 * This interface defines an abstract listener for a parse tree produced by SimpleBooleanParser.
 */
class  SimpleBooleanListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterParse(SimpleBooleanParser::ParseContext *ctx) = 0;
  virtual void exitParse(SimpleBooleanParser::ParseContext *ctx) = 0;

  virtual void enterBinaryExpression(SimpleBooleanParser::BinaryExpressionContext *ctx) = 0;
  virtual void exitBinaryExpression(SimpleBooleanParser::BinaryExpressionContext *ctx) = 0;

  virtual void enterDecimalExpression(SimpleBooleanParser::DecimalExpressionContext *ctx) = 0;
  virtual void exitDecimalExpression(SimpleBooleanParser::DecimalExpressionContext *ctx) = 0;

  virtual void enterBoolExpression(SimpleBooleanParser::BoolExpressionContext *ctx) = 0;
  virtual void exitBoolExpression(SimpleBooleanParser::BoolExpressionContext *ctx) = 0;

  virtual void enterIdentifierExpression(SimpleBooleanParser::IdentifierExpressionContext *ctx) = 0;
  virtual void exitIdentifierExpression(SimpleBooleanParser::IdentifierExpressionContext *ctx) = 0;

  virtual void enterNotExpression(SimpleBooleanParser::NotExpressionContext *ctx) = 0;
  virtual void exitNotExpression(SimpleBooleanParser::NotExpressionContext *ctx) = 0;

  virtual void enterParenExpression(SimpleBooleanParser::ParenExpressionContext *ctx) = 0;
  virtual void exitParenExpression(SimpleBooleanParser::ParenExpressionContext *ctx) = 0;

  virtual void enterComparatorExpression(SimpleBooleanParser::ComparatorExpressionContext *ctx) = 0;
  virtual void exitComparatorExpression(SimpleBooleanParser::ComparatorExpressionContext *ctx) = 0;

  virtual void enterComparator(SimpleBooleanParser::ComparatorContext *ctx) = 0;
  virtual void exitComparator(SimpleBooleanParser::ComparatorContext *ctx) = 0;

  virtual void enterBinary(SimpleBooleanParser::BinaryContext *ctx) = 0;
  virtual void exitBinary(SimpleBooleanParser::BinaryContext *ctx) = 0;

  virtual void enterBoolean(SimpleBooleanParser::BooleanContext *ctx) = 0;
  virtual void exitBoolean(SimpleBooleanParser::BooleanContext *ctx) = 0;


};

