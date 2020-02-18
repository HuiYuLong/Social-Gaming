
// Generated from SimpleBoolean.g by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"
#include "SimpleBooleanVisitor.h"


namespace antlrcpp {

/**
 * This class provides an empty implementation of SimpleBooleanVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  SimpleBooleanBaseVisitor : public SimpleBooleanVisitor {
public:

  virtual antlrcpp::Any visitParse(SimpleBooleanParser::ParseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBinaryExpression(SimpleBooleanParser::BinaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDecimalExpression(SimpleBooleanParser::DecimalExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBoolExpression(SimpleBooleanParser::BoolExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIdentifierExpression(SimpleBooleanParser::IdentifierExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNotExpression(SimpleBooleanParser::NotExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitParenExpression(SimpleBooleanParser::ParenExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitComparatorExpression(SimpleBooleanParser::ComparatorExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitComparator(SimpleBooleanParser::ComparatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBinary(SimpleBooleanParser::BinaryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBoolean(SimpleBooleanParser::BooleanContext *ctx) override {
    return visitChildren(ctx);
  }


};

}  // namespace antlrcpp
