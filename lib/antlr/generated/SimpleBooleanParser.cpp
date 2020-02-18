
// Generated from SimpleBoolean.g4 by ANTLR 4.8


#include "SimpleBooleanListener.h"

#include "SimpleBooleanParser.h"


using namespace antlrcpp;
using namespace antlr4;

SimpleBooleanParser::SimpleBooleanParser(TokenStream *input) : Parser(input) {
  _interpreter = new atn::ParserATNSimulator(this, _atn, _decisionToDFA, _sharedContextCache);
}

SimpleBooleanParser::~SimpleBooleanParser() {
  delete _interpreter;
}

std::string SimpleBooleanParser::getGrammarFileName() const {
  return "SimpleBoolean.g4";
}

const std::vector<std::string>& SimpleBooleanParser::getRuleNames() const {
  return _ruleNames;
}

dfa::Vocabulary& SimpleBooleanParser::getVocabulary() const {
  return _vocabulary;
}


//----------------- ParseContext ------------------------------------------------------------------

SimpleBooleanParser::ParseContext::ParseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SimpleBooleanParser::ExpressionContext* SimpleBooleanParser::ParseContext::expression() {
  return getRuleContext<SimpleBooleanParser::ExpressionContext>(0);
}

tree::TerminalNode* SimpleBooleanParser::ParseContext::EOF() {
  return getToken(SimpleBooleanParser::EOF, 0);
}


size_t SimpleBooleanParser::ParseContext::getRuleIndex() const {
  return SimpleBooleanParser::RuleParse;
}

void SimpleBooleanParser::ParseContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SimpleBooleanListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterParse(this);
}

void SimpleBooleanParser::ParseContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SimpleBooleanListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitParse(this);
}

SimpleBooleanParser::ParseContext* SimpleBooleanParser::parse() {
  ParseContext *_localctx = _tracker.createInstance<ParseContext>(_ctx, getState());
  enterRule(_localctx, 0, SimpleBooleanParser::RuleParse);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(10);
    expression(0);
    setState(11);
    match(SimpleBooleanParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpressionContext ------------------------------------------------------------------

SimpleBooleanParser::ExpressionContext::ExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SimpleBooleanParser::ExpressionContext::getRuleIndex() const {
  return SimpleBooleanParser::RuleExpression;
}

void SimpleBooleanParser::ExpressionContext::copyFrom(ExpressionContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- BinaryExpressionContext ------------------------------------------------------------------

std::vector<SimpleBooleanParser::ExpressionContext *> SimpleBooleanParser::BinaryExpressionContext::expression() {
  return getRuleContexts<SimpleBooleanParser::ExpressionContext>();
}

SimpleBooleanParser::ExpressionContext* SimpleBooleanParser::BinaryExpressionContext::expression(size_t i) {
  return getRuleContext<SimpleBooleanParser::ExpressionContext>(i);
}

SimpleBooleanParser::BinaryContext* SimpleBooleanParser::BinaryExpressionContext::binary() {
  return getRuleContext<SimpleBooleanParser::BinaryContext>(0);
}

SimpleBooleanParser::BinaryExpressionContext::BinaryExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }

void SimpleBooleanParser::BinaryExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SimpleBooleanListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBinaryExpression(this);
}
void SimpleBooleanParser::BinaryExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SimpleBooleanListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBinaryExpression(this);
}
//----------------- DecimalExpressionContext ------------------------------------------------------------------

tree::TerminalNode* SimpleBooleanParser::DecimalExpressionContext::DECIMAL() {
  return getToken(SimpleBooleanParser::DECIMAL, 0);
}

SimpleBooleanParser::DecimalExpressionContext::DecimalExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }

void SimpleBooleanParser::DecimalExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SimpleBooleanListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDecimalExpression(this);
}
void SimpleBooleanParser::DecimalExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SimpleBooleanListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDecimalExpression(this);
}
//----------------- BoolExpressionContext ------------------------------------------------------------------

SimpleBooleanParser::BooleanContext* SimpleBooleanParser::BoolExpressionContext::boolean() {
  return getRuleContext<SimpleBooleanParser::BooleanContext>(0);
}

SimpleBooleanParser::BoolExpressionContext::BoolExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }

void SimpleBooleanParser::BoolExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SimpleBooleanListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBoolExpression(this);
}
void SimpleBooleanParser::BoolExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SimpleBooleanListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBoolExpression(this);
}
//----------------- IdentifierExpressionContext ------------------------------------------------------------------

tree::TerminalNode* SimpleBooleanParser::IdentifierExpressionContext::IDENTIFIER() {
  return getToken(SimpleBooleanParser::IDENTIFIER, 0);
}

SimpleBooleanParser::IdentifierExpressionContext::IdentifierExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }

void SimpleBooleanParser::IdentifierExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SimpleBooleanListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterIdentifierExpression(this);
}
void SimpleBooleanParser::IdentifierExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SimpleBooleanListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitIdentifierExpression(this);
}
//----------------- NotExpressionContext ------------------------------------------------------------------

tree::TerminalNode* SimpleBooleanParser::NotExpressionContext::NOT() {
  return getToken(SimpleBooleanParser::NOT, 0);
}

SimpleBooleanParser::ExpressionContext* SimpleBooleanParser::NotExpressionContext::expression() {
  return getRuleContext<SimpleBooleanParser::ExpressionContext>(0);
}

SimpleBooleanParser::NotExpressionContext::NotExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }

void SimpleBooleanParser::NotExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SimpleBooleanListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNotExpression(this);
}
void SimpleBooleanParser::NotExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SimpleBooleanListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNotExpression(this);
}
//----------------- ParenExpressionContext ------------------------------------------------------------------

tree::TerminalNode* SimpleBooleanParser::ParenExpressionContext::LPAREN() {
  return getToken(SimpleBooleanParser::LPAREN, 0);
}

SimpleBooleanParser::ExpressionContext* SimpleBooleanParser::ParenExpressionContext::expression() {
  return getRuleContext<SimpleBooleanParser::ExpressionContext>(0);
}

tree::TerminalNode* SimpleBooleanParser::ParenExpressionContext::RPAREN() {
  return getToken(SimpleBooleanParser::RPAREN, 0);
}

SimpleBooleanParser::ParenExpressionContext::ParenExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }

void SimpleBooleanParser::ParenExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SimpleBooleanListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterParenExpression(this);
}
void SimpleBooleanParser::ParenExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SimpleBooleanListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitParenExpression(this);
}
//----------------- ComparatorExpressionContext ------------------------------------------------------------------

std::vector<SimpleBooleanParser::ExpressionContext *> SimpleBooleanParser::ComparatorExpressionContext::expression() {
  return getRuleContexts<SimpleBooleanParser::ExpressionContext>();
}

SimpleBooleanParser::ExpressionContext* SimpleBooleanParser::ComparatorExpressionContext::expression(size_t i) {
  return getRuleContext<SimpleBooleanParser::ExpressionContext>(i);
}

SimpleBooleanParser::ComparatorContext* SimpleBooleanParser::ComparatorExpressionContext::comparator() {
  return getRuleContext<SimpleBooleanParser::ComparatorContext>(0);
}

SimpleBooleanParser::ComparatorExpressionContext::ComparatorExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }

void SimpleBooleanParser::ComparatorExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SimpleBooleanListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterComparatorExpression(this);
}
void SimpleBooleanParser::ComparatorExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SimpleBooleanListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitComparatorExpression(this);
}

SimpleBooleanParser::ExpressionContext* SimpleBooleanParser::expression() {
   return expression(0);
}

SimpleBooleanParser::ExpressionContext* SimpleBooleanParser::expression(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SimpleBooleanParser::ExpressionContext *_localctx = _tracker.createInstance<ExpressionContext>(_ctx, parentState);
  SimpleBooleanParser::ExpressionContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 2;
  enterRecursionRule(_localctx, 2, SimpleBooleanParser::RuleExpression, precedence);

    

  auto onExit = finally([=] {
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(23);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SimpleBooleanParser::LPAREN: {
        _localctx = _tracker.createInstance<ParenExpressionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;

        setState(14);
        match(SimpleBooleanParser::LPAREN);
        setState(15);
        expression(0);
        setState(16);
        match(SimpleBooleanParser::RPAREN);
        break;
      }

      case SimpleBooleanParser::NOT: {
        _localctx = _tracker.createInstance<NotExpressionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(18);
        match(SimpleBooleanParser::NOT);
        setState(19);
        expression(6);
        break;
      }

      case SimpleBooleanParser::TRUE:
      case SimpleBooleanParser::FALSE: {
        _localctx = _tracker.createInstance<BoolExpressionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(20);
        boolean();
        break;
      }

      case SimpleBooleanParser::IDENTIFIER: {
        _localctx = _tracker.createInstance<IdentifierExpressionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(21);
        match(SimpleBooleanParser::IDENTIFIER);
        break;
      }

      case SimpleBooleanParser::DECIMAL: {
        _localctx = _tracker.createInstance<DecimalExpressionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(22);
        match(SimpleBooleanParser::DECIMAL);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(35);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(33);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<ComparatorExpressionContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          newContext->left = previousContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(25);

          if (!(precpred(_ctx, 5))) throw FailedPredicateException(this, "precpred(_ctx, 5)");
          setState(26);
          dynamic_cast<ComparatorExpressionContext *>(_localctx)->op = comparator();
          setState(27);
          dynamic_cast<ComparatorExpressionContext *>(_localctx)->right = expression(6);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<BinaryExpressionContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          newContext->left = previousContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(29);

          if (!(precpred(_ctx, 4))) throw FailedPredicateException(this, "precpred(_ctx, 4)");
          setState(30);
          dynamic_cast<BinaryExpressionContext *>(_localctx)->op = binary();
          setState(31);
          dynamic_cast<BinaryExpressionContext *>(_localctx)->right = expression(5);
          break;
        }

        } 
      }
      setState(37);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- ComparatorContext ------------------------------------------------------------------

SimpleBooleanParser::ComparatorContext::ComparatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SimpleBooleanParser::ComparatorContext::GT() {
  return getToken(SimpleBooleanParser::GT, 0);
}

tree::TerminalNode* SimpleBooleanParser::ComparatorContext::GE() {
  return getToken(SimpleBooleanParser::GE, 0);
}

tree::TerminalNode* SimpleBooleanParser::ComparatorContext::LT() {
  return getToken(SimpleBooleanParser::LT, 0);
}

tree::TerminalNode* SimpleBooleanParser::ComparatorContext::LE() {
  return getToken(SimpleBooleanParser::LE, 0);
}

tree::TerminalNode* SimpleBooleanParser::ComparatorContext::EQ() {
  return getToken(SimpleBooleanParser::EQ, 0);
}


size_t SimpleBooleanParser::ComparatorContext::getRuleIndex() const {
  return SimpleBooleanParser::RuleComparator;
}

void SimpleBooleanParser::ComparatorContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SimpleBooleanListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterComparator(this);
}

void SimpleBooleanParser::ComparatorContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SimpleBooleanListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitComparator(this);
}

SimpleBooleanParser::ComparatorContext* SimpleBooleanParser::comparator() {
  ComparatorContext *_localctx = _tracker.createInstance<ComparatorContext>(_ctx, getState());
  enterRule(_localctx, 4, SimpleBooleanParser::RuleComparator);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(38);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << SimpleBooleanParser::GT)
      | (1ULL << SimpleBooleanParser::GE)
      | (1ULL << SimpleBooleanParser::LT)
      | (1ULL << SimpleBooleanParser::LE)
      | (1ULL << SimpleBooleanParser::EQ))) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BinaryContext ------------------------------------------------------------------

SimpleBooleanParser::BinaryContext::BinaryContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SimpleBooleanParser::BinaryContext::AND() {
  return getToken(SimpleBooleanParser::AND, 0);
}

tree::TerminalNode* SimpleBooleanParser::BinaryContext::OR() {
  return getToken(SimpleBooleanParser::OR, 0);
}


size_t SimpleBooleanParser::BinaryContext::getRuleIndex() const {
  return SimpleBooleanParser::RuleBinary;
}

void SimpleBooleanParser::BinaryContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SimpleBooleanListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBinary(this);
}

void SimpleBooleanParser::BinaryContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SimpleBooleanListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBinary(this);
}

SimpleBooleanParser::BinaryContext* SimpleBooleanParser::binary() {
  BinaryContext *_localctx = _tracker.createInstance<BinaryContext>(_ctx, getState());
  enterRule(_localctx, 6, SimpleBooleanParser::RuleBinary);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(40);
    _la = _input->LA(1);
    if (!(_la == SimpleBooleanParser::AND

    || _la == SimpleBooleanParser::OR)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BooleanContext ------------------------------------------------------------------

SimpleBooleanParser::BooleanContext::BooleanContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SimpleBooleanParser::BooleanContext::TRUE() {
  return getToken(SimpleBooleanParser::TRUE, 0);
}

tree::TerminalNode* SimpleBooleanParser::BooleanContext::FALSE() {
  return getToken(SimpleBooleanParser::FALSE, 0);
}


size_t SimpleBooleanParser::BooleanContext::getRuleIndex() const {
  return SimpleBooleanParser::RuleBoolean;
}

void SimpleBooleanParser::BooleanContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SimpleBooleanListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBoolean(this);
}

void SimpleBooleanParser::BooleanContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SimpleBooleanListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBoolean(this);
}

SimpleBooleanParser::BooleanContext* SimpleBooleanParser::boolean() {
  BooleanContext *_localctx = _tracker.createInstance<BooleanContext>(_ctx, getState());
  enterRule(_localctx, 8, SimpleBooleanParser::RuleBoolean);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(42);
    _la = _input->LA(1);
    if (!(_la == SimpleBooleanParser::TRUE

    || _la == SimpleBooleanParser::FALSE)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

bool SimpleBooleanParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 1: return expressionSempred(dynamic_cast<ExpressionContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool SimpleBooleanParser::expressionSempred(ExpressionContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 5);
    case 1: return precpred(_ctx, 4);

  default:
    break;
  }
  return true;
}

// Static vars and initialization.
std::vector<dfa::DFA> SimpleBooleanParser::_decisionToDFA;
atn::PredictionContextCache SimpleBooleanParser::_sharedContextCache;

// We own the ATN which in turn owns the ATN states.
atn::ATN SimpleBooleanParser::_atn;
std::vector<uint16_t> SimpleBooleanParser::_serializedATN;

std::vector<std::string> SimpleBooleanParser::_ruleNames = {
  "parse", "expression", "comparator", "binary", "boolean"
};

std::vector<std::string> SimpleBooleanParser::_literalNames = {
  "", "'&&'", "'||'", "'!'", "'true'", "'false'", "'>'", "'>='", "'<'", 
  "'<='", "'=='", "'('", "')'"
};

std::vector<std::string> SimpleBooleanParser::_symbolicNames = {
  "", "AND", "OR", "NOT", "TRUE", "FALSE", "GT", "GE", "LT", "LE", "EQ", 
  "LPAREN", "RPAREN", "DECIMAL", "IDENTIFIER", "WS"
};

dfa::Vocabulary SimpleBooleanParser::_vocabulary(_literalNames, _symbolicNames);

std::vector<std::string> SimpleBooleanParser::_tokenNames;

SimpleBooleanParser::Initializer::Initializer() {
	for (size_t i = 0; i < _symbolicNames.size(); ++i) {
		std::string name = _vocabulary.getLiteralName(i);
		if (name.empty()) {
			name = _vocabulary.getSymbolicName(i);
		}

		if (name.empty()) {
			_tokenNames.push_back("<INVALID>");
		} else {
      _tokenNames.push_back(name);
    }
	}

  _serializedATN = {
    0x3, 0x608b, 0xa72a, 0x8133, 0xb9ed, 0x417c, 0x3be7, 0x7786, 0x5964, 
    0x3, 0x11, 0x2f, 0x4, 0x2, 0x9, 0x2, 0x4, 0x3, 0x9, 0x3, 0x4, 0x4, 0x9, 
    0x4, 0x4, 0x5, 0x9, 0x5, 0x4, 0x6, 0x9, 0x6, 0x3, 0x2, 0x3, 0x2, 0x3, 
    0x2, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 
    0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x5, 0x3, 0x1a, 0xa, 0x3, 0x3, 0x3, 
    0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 
    0x7, 0x3, 0x24, 0xa, 0x3, 0xc, 0x3, 0xe, 0x3, 0x27, 0xb, 0x3, 0x3, 0x4, 
    0x3, 0x4, 0x3, 0x5, 0x3, 0x5, 0x3, 0x6, 0x3, 0x6, 0x3, 0x6, 0x2, 0x3, 
    0x4, 0x7, 0x2, 0x4, 0x6, 0x8, 0xa, 0x2, 0x5, 0x3, 0x2, 0x8, 0xc, 0x3, 
    0x2, 0x3, 0x4, 0x3, 0x2, 0x6, 0x7, 0x2, 0x2f, 0x2, 0xc, 0x3, 0x2, 0x2, 
    0x2, 0x4, 0x19, 0x3, 0x2, 0x2, 0x2, 0x6, 0x28, 0x3, 0x2, 0x2, 0x2, 0x8, 
    0x2a, 0x3, 0x2, 0x2, 0x2, 0xa, 0x2c, 0x3, 0x2, 0x2, 0x2, 0xc, 0xd, 0x5, 
    0x4, 0x3, 0x2, 0xd, 0xe, 0x7, 0x2, 0x2, 0x3, 0xe, 0x3, 0x3, 0x2, 0x2, 
    0x2, 0xf, 0x10, 0x8, 0x3, 0x1, 0x2, 0x10, 0x11, 0x7, 0xd, 0x2, 0x2, 
    0x11, 0x12, 0x5, 0x4, 0x3, 0x2, 0x12, 0x13, 0x7, 0xe, 0x2, 0x2, 0x13, 
    0x1a, 0x3, 0x2, 0x2, 0x2, 0x14, 0x15, 0x7, 0x5, 0x2, 0x2, 0x15, 0x1a, 
    0x5, 0x4, 0x3, 0x8, 0x16, 0x1a, 0x5, 0xa, 0x6, 0x2, 0x17, 0x1a, 0x7, 
    0x10, 0x2, 0x2, 0x18, 0x1a, 0x7, 0xf, 0x2, 0x2, 0x19, 0xf, 0x3, 0x2, 
    0x2, 0x2, 0x19, 0x14, 0x3, 0x2, 0x2, 0x2, 0x19, 0x16, 0x3, 0x2, 0x2, 
    0x2, 0x19, 0x17, 0x3, 0x2, 0x2, 0x2, 0x19, 0x18, 0x3, 0x2, 0x2, 0x2, 
    0x1a, 0x25, 0x3, 0x2, 0x2, 0x2, 0x1b, 0x1c, 0xc, 0x7, 0x2, 0x2, 0x1c, 
    0x1d, 0x5, 0x6, 0x4, 0x2, 0x1d, 0x1e, 0x5, 0x4, 0x3, 0x8, 0x1e, 0x24, 
    0x3, 0x2, 0x2, 0x2, 0x1f, 0x20, 0xc, 0x6, 0x2, 0x2, 0x20, 0x21, 0x5, 
    0x8, 0x5, 0x2, 0x21, 0x22, 0x5, 0x4, 0x3, 0x7, 0x22, 0x24, 0x3, 0x2, 
    0x2, 0x2, 0x23, 0x1b, 0x3, 0x2, 0x2, 0x2, 0x23, 0x1f, 0x3, 0x2, 0x2, 
    0x2, 0x24, 0x27, 0x3, 0x2, 0x2, 0x2, 0x25, 0x23, 0x3, 0x2, 0x2, 0x2, 
    0x25, 0x26, 0x3, 0x2, 0x2, 0x2, 0x26, 0x5, 0x3, 0x2, 0x2, 0x2, 0x27, 
    0x25, 0x3, 0x2, 0x2, 0x2, 0x28, 0x29, 0x9, 0x2, 0x2, 0x2, 0x29, 0x7, 
    0x3, 0x2, 0x2, 0x2, 0x2a, 0x2b, 0x9, 0x3, 0x2, 0x2, 0x2b, 0x9, 0x3, 
    0x2, 0x2, 0x2, 0x2c, 0x2d, 0x9, 0x4, 0x2, 0x2, 0x2d, 0xb, 0x3, 0x2, 
    0x2, 0x2, 0x5, 0x19, 0x23, 0x25, 
  };

  atn::ATNDeserializer deserializer;
  _atn = deserializer.deserialize(_serializedATN);

  size_t count = _atn.getNumberOfDecisions();
  _decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    _decisionToDFA.emplace_back(_atn.getDecisionState(i), i);
  }
}

SimpleBooleanParser::Initializer SimpleBooleanParser::_init;
