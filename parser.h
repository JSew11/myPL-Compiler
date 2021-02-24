//----------------------------------------------------------------------
// NAME: Joshua Seward
// FILE: parser.h
// DATE: 2/15/2021
// DESC: Parser for myPL using given grammar rules
//----------------------------------------------------------------------

#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "mypl_exception.h"
#include "ast.h"


class Parser
{
public:

  // create a new recursive descent parser
  Parser(const Lexer& program_lexer);

  // run the parser
  void parse(Program& node);
  
private:
  Lexer lexer;
  Token curr_token;
  
  // helper functions
  void advance();
  void eat(TokenType t, std::string err_msg);
  void error(std::string err_msg);
  bool is_operator(TokenType t);
  
  // recursive descent functions
  void tdecl(TypeDecl*& tDec);
  void vdecls(VarDeclStmt*& vDecStmt);
  void fdecl(FunDecl*& fDec);
  void params(FunDecl*& fDec);
  void dtype(Token& dType);
  void stmts(std::list<Stmt*>& stamts);
  void stmt(std::list<Stmt*>& stamts);
  void vdecl_stmt(VarDeclStmt*& vDecStmt);
  void assign_stmt(AssignStmt*& aStmt);
  void lvalue(std::list<Token>& id_list);
  void cond_stmt(IfStmt*& iStmt);
  void condt(IfStmt*& iStmt);
  void while_stmt(WhileStmt*& wStmt);
  void for_stmt(ForStmt*& fStmt);
  void call_expr(CallExpr*& cExpr);
  void args(std::list<Expr*> arg_list);
  void exit_stmt(ReturnStmt*& rStmt);
  void expr(Expr*& exp);
  void op(Token& oper);
  void rvalue(RValue*& rVal);
  void pval(Token& value);
  void idrval(IDRValue*& idrVal);
};


// constructor
Parser::Parser(const Lexer& program_lexer) : lexer(program_lexer)
{
}


// Helper functions

void Parser::advance()
{
  curr_token = lexer.next_token();
}


void Parser::eat(TokenType t, std::string err_msg)
{
  if (curr_token.type() == t)
    advance();
  else
    error(err_msg);
}


void Parser::error(std::string err_msg)
{
  std::string s = err_msg + "found '" + curr_token.lexeme() + "'";
  int line = curr_token.line();
  int col = curr_token.column();
  throw MyPLException(SYNTAX, s, line, col);
}


bool Parser::is_operator(TokenType t)
{
  return t == PLUS or t == MINUS or t == DIVIDE or t == MULTIPLY or
    t == MODULO or t == AND or t == OR or t == EQUAL or t == LESS or
    t == GREATER or t == LESS_EQUAL or t == GREATER_EQUAL or t == NOT_EQUAL;
}


// Recursive-decent functions

void Parser::parse(Program& node)
{
  advance();
  while (curr_token.type() != EOS) {
    if (curr_token.type() == TYPE){
      TypeDecl* tdec = new TypeDecl();
      tdec->id = curr_token;
      tdecl(tdec);
    } else if(curr_token.type() == FUN){
      FunDecl* fdec = new FunDecl();
      fdecl(fdec);
    }
  }
  eat(EOS, "expecting end-of-file ");
}


void Parser::tdecl(TypeDecl*& tDec)
{
  advance();
  while(curr_token.type() == VAR){
    VarDeclStmt* vDecStmt = new VarDeclStmt();
    *vDecStmt->type = tDec->id;
    vdecls(vDecStmt);
    tDec->vdecls.push_back(vDecStmt);
  }
  eat(END, "expecting 'END' keyword ");
}

void Parser::vdecls(VarDeclStmt*& vDecStmt){
  vdecl_stmt(vDecStmt);
}

void Parser::fdecl(FunDecl*& fdec)
{
  advance();
  if(curr_token.type() == NIL){
    fdec->return_type = curr_token;
    advance();
  }
  else if(curr_token.type() == INT_TYPE ||
  curr_token.type() == DOUBLE_TYPE ||
  curr_token.type() == BOOL_TYPE ||
  curr_token.type() == CHAR_TYPE ||
  curr_token.type() == STRING_TYPE ||
  curr_token.type() == ID){
    dtype(fdec->return_type);
  }
  else error("invalid function declaration ");
  if(curr_token.type() == ID){
    fdec->id = curr_token;
    advance();
  }
  else error("expecting variable ID ");
  eat(LPAREN, "expecting '(' ");
  params(fdec);
  eat(RPAREN, "expecting ')' ");
  stmts(fdec->stmts);
  eat(END, "expecting 'END' keyword ");
}

void Parser::params(FunDecl*& fdecl){
  if(curr_token.type() == ID){
    FunDecl::FunParam p;
    p.id = curr_token;
    advance();
    eat(COLON, "expecting \':\' ");
    dtype(p.type);
    fdecl->params.push_back(p);
    while(curr_token.type() == COMMA){
      advance();
      if(curr_token.type() == ID){
        p.id = curr_token;
      }
      else error("expecting id ");
      eat(COLON, "expecting \':\' ");
      dtype(p.type);
      fdecl->params.push_back(p);
    }
  }
}

void Parser::dtype(Token& dType){
  if(curr_token.type() == INT_TYPE ||
    curr_token.type() == DOUBLE_TYPE ||
    curr_token.type() == BOOL_TYPE ||
    curr_token.type() == CHAR_TYPE ||
    curr_token.type() == STRING_TYPE ||
    curr_token.type() == ID){
    dType = curr_token;
    advance();
  }
}

void Parser::stmts(std::list<Stmt*>& stamts){
  if(curr_token.type() == VAR ||
  curr_token.type() == ID ||
  curr_token.type() == IF ||
  curr_token.type() == WHILE ||
  curr_token.type() == FOR ||
  curr_token.type() == RETURN){
    stmt(stamts);
    stmts(stamts);
  }
}

void Parser::stmt(std::list<Stmt*>& stamts){
  if(curr_token.type() == VAR){
    VarDeclStmt* vDecStmt = new VarDeclStmt();
    vdecl_stmt(vDecStmt);
    stamts.push_back(vDecStmt);
  }
  else if(curr_token.type() == ID){
    Token* id = new Token(curr_token.type(), curr_token.lexeme(), 
                          curr_token.line(), curr_token.column());
    advance();
    if(curr_token.type() == LPAREN){
      CallExpr* cExpr = new CallExpr();
      cExpr->function_id = *id;
      call_expr(cExpr);
      stamts.push_back(cExpr);
    }
    else{
      AssignStmt* aStmt = new AssignStmt();
      aStmt->lvalue_list.push_back(*id);
      assign_stmt(aStmt);
      stamts.push_back(aStmt);
    }
  }
  else if(curr_token.type() == IF){
    IfStmt* iStmt = new IfStmt();
    cond_stmt(iStmt);
    stamts.push_back(iStmt);
  }
  else if(curr_token.type() == WHILE){
    WhileStmt* wStmt = new WhileStmt();
    while_stmt(wStmt);
    stamts.push_back(wStmt);
  }
  else if(curr_token.type() == FOR){
    ForStmt* fStmt = new ForStmt();
    for_stmt(fStmt);
    stamts.push_back(fStmt);
  }
  else if(curr_token.type() == RETURN){
    ReturnStmt* rStmt = new ReturnStmt();
    exit_stmt(rStmt);
    stamts.push_back(rStmt);
  }
}

void Parser::vdecl_stmt(VarDeclStmt*& vDecStmt){
  advance();
  vDecStmt->id = curr_token;
  eat(ID, "expecting id ");
  if(curr_token.type() == COLON){
    advance();
    dtype(*vDecStmt->type);
  }
  eat(ASSIGN, "expecting '=' ");
}

void Parser::assign_stmt(AssignStmt*& aStmt){
  lvalue(aStmt->lvalue_list);
  eat(ASSIGN, "expecting '=' ");
  expr(aStmt->expr);
}

void Parser::lvalue(std::list<Token>& id_list){
  if(curr_token.type() == ID){
    id_list.push_back(curr_token);
    advance();
  }
  else error("expecting id ");
  while(curr_token.type() == DOT){
    advance();
    if(curr_token.type() == ID){
      id_list.push_back(curr_token);
      advance();
    }
    else error("expecting id ");
  }
}

void Parser::cond_stmt(IfStmt*& iStmt){
  advance();;
  expr(iStmt->if_part->expr);
  eat(THEN, "expecting 'then' keyword ");
  stmts(iStmt->if_part->stmts);
  condt(iStmt);
  eat(END, "expecting 'end' keyword ");
}

void Parser::condt(IfStmt*& iStmt){
  if(curr_token.type() == ELSEIF){
    advance();
    BasicIf* else_if = new BasicIf();
    expr(else_if->expr);
    eat(THEN, "expecting 'then' keyword ");
    stmts(else_if->stmts);
    iStmt->else_ifs.push_back(else_if);
    condt(iStmt);
  }
  if(curr_token.type() == ELSE){
    advance();
    stmts(iStmt->body_stmts);
  }
}

void Parser::while_stmt(WhileStmt*& wStmt){
  eat(WHILE, "expecting 'while' keyword");
  expr(wStmt->expr);
  eat(DO, "expecting 'do' keyword ");
  stmts(wStmt->stmts);
  eat(END, "expecting 'end' keyword ");
}

void Parser::for_stmt(ForStmt*& fStmt){
  eat(FOR, "expecting 'for' keyword ");
  if(curr_token.type() == ID){
    fStmt->var_id = curr_token;
    advance();
  }
  else error("expecting id ");
  eat(ASSIGN, "expecting '=' ");
  expr(fStmt->start);
  eat(TO, "expecting 'to' keyword ");
  expr(fStmt->end); 
  eat(DO, "expecting 'do' keyword ");
  stmts(fStmt->stmts);
  eat(END, "expecting 'end' keyword");
}

void Parser::call_expr(CallExpr*& cExpr){
  eat(LPAREN, "expecting '(' ");
  args(cExpr->arg_list);
  eat(RPAREN, "expecting ')' ");
}

void Parser::args(std::list<Expr*> arg_list){
  if(curr_token.type() == NOT ||
  curr_token.type() == LPAREN ||
  curr_token.type() == NIL ||
  curr_token.type() == NEW || 
  curr_token.type() == NEG ||
  curr_token.type() == INT_VAL ||
  curr_token.type() == DOUBLE_VAL ||
  curr_token.type() == BOOL_VAL ||
  curr_token.type() == CHAR_VAL ||
  curr_token.type() == STRING_VAL ||
  curr_token.type() == ID){
    Expr* exp = new Expr();
    expr(exp);
    arg_list.push_back(exp);
    while(curr_token.type() == COMMA){
      advance();
      expr(exp);
      arg_list.push_back(exp);
    }
  }
}

void Parser::exit_stmt(ReturnStmt*& rStmt){
  advance();
  expr(rStmt->expr);
}

void Parser::expr(Expr*& exp){
  if(curr_token.type() == NOT){
    exp->negated = true;
    advance();
    ComplexTerm* cTerm = new ComplexTerm();
    expr(cTerm->expr);
    exp->first = cTerm;
  }
  else if(curr_token.type() == LPAREN){
    advance();
    ComplexTerm* cTerm = new ComplexTerm();
    expr(cTerm->expr);
    exp->first = cTerm;
    eat(RPAREN, "expecting ')' ");
  }
  else{
    SimpleTerm* sTerm = new SimpleTerm();
    rvalue(sTerm->rvalue);
    exp->first = sTerm;
  }
  if(is_operator(curr_token.type())){
    op(*exp->op);
    expr(exp->rest);
  }
}

void Parser::op(Token& oper){
  if(is_operator(curr_token.type())){
    oper = curr_token;
    advance();
  }
}

void Parser::rvalue(RValue*& rVal){
  if(curr_token.type() == INT_VAL || 
  curr_token.type() == DOUBLE_VAL ||
  curr_token.type() == BOOL_VAL ||
  curr_token.type() == CHAR_VAL ||
  curr_token.type() == STRING_VAL){
    SimpleRValue* sRVal = new SimpleRValue();
    pval(sRVal->value);
    rVal = sRVal;
  }
  else if(curr_token.type() == NIL)
    advance();
  else if(curr_token.type() == NEW){
    advance();
    eat(ID, "expecting id ");
  }
  else if(curr_token.type() == ID){
    Token* id = new Token(curr_token.type(), curr_token.lexeme(),
                         curr_token.line(), curr_token.column());
    advance();
    if(curr_token.type() == LPAREN){
      CallExpr* cExpr = new CallExpr();
      cExpr->function_id = *id;
      call_expr(cExpr);
      rVal = cExpr;
    }
    else{
      IDRValue* idrVal = new IDRValue();
      idrval(idrVal);
    }
  }
  else if(curr_token.type() == NEG){
    advance();
    NegatedRValue* nRVal = new NegatedRValue();
    expr(nRVal->expr);
    rVal = nRVal;
  }  
}

void Parser::pval(Token& value){
  if(curr_token.type() == INT_VAL || 
  curr_token.type() == DOUBLE_VAL ||
  curr_token.type() == BOOL_VAL ||
  curr_token.type() == CHAR_VAL ||
  curr_token.type() == STRING_VAL){
    value = curr_token;
    advance();
  }
  else error("expecting value ");
}

void Parser::idrval(IDRValue*& idrVal){
  while(curr_token.type() == DOT){
    advance();
    if(curr_token.type() == ID){
      idrVal->path.push_back(curr_token);
      advance();
    }
    else error("expecting id ");
  }
}

#endif
