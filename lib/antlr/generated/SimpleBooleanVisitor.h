
// Generated from SimpleBoolean.g by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"
#include "SimpleBooleanParser.h"


namespace antlrcpp {

/**
 * This class defines an abstract visitor for a parse tree
 * produced by SimpleBooleanParser.
 */
class  SimpleBooleanVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by SimpleBooleanParser.
   */
    virtual antlrcpp::Any visitParse(SimpleBooleanParser::ParseContext *context) = 0;

    virtual antlrcpp::Any visitBinaryExpression(SimpleBooleanParser::BinaryExpressionContext *context) = 0;

    virtual antlrcpp::Any visitDecimalExpression(SimpleBooleanParser::DecimalExpressionContext *context) = 0;

    virtual antlrcpp::Any visitBoolExpression(SimpleBooleanParser::BoolExpressionContext *context) = 0;

    virtual antlrcpp::Any visitIdentifierExpression(SimpleBooleanParser::IdentifierExpressionContext *context) = 0;

    virtual antlrcpp::Any visitNotExpression(SimpleBooleanParser::NotExpressionContext *context) = 0;

    virtual antlrcpp::Any visitParenExpression(SimpleBooleanParser::ParenExpressionContext *context) = 0;

    virtual antlrcpp::Any visitComparatorExpression(SimpleBooleanParser::ComparatorExpressionContext *context) = 0;

    virtual antlrcpp::Any visitComparator(SimpleBooleanParser::ComparatorContext *context) = 0;

    virtual antlrcpp::Any visitBinary(SimpleBooleanParser::BinaryContext *context) = 0;

    virtual antlrcpp::Any visitBoolean(SimpleBooleanParser::BooleanContext *context) = 0;


};

}  // namespace antlrcpp
