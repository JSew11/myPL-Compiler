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
  void parse(Program& prog);
  
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
  void vdecls(std::list<VarDeclStmt*>& vdecs);
  void fdecl(FunDecl*& fDec);
  void params(std::list<FunDecl::FunParam>& ps);
  void dtype(Token& dType);
  void stmts(std::list<Stmt*>& stms);
  void stmt(std::list<Stmt*>& stms);
  void vdecl_stmt(VarDeclStmt*& vDecl);
  void assign_stmt(AssignStmt*& aStmt);
  void lvalue(std::list<Token>& lvalue_list);
  void cond_stmt(IfStmt*& iStmt);
  void condt(IfStmt*& iStmt);
  void while_stmt(WhileStmt*& wStmt);
  void for_stmt(ForStmt*& fStmt);
  void call_expr(CallExpr*& cExpr);
  void args(std::list<Expr*>& arg_list);
  void exit_stmt(ReturnStmt*& rStmt);
  void expr(Expr*& exp);
  void op(Token*& op);
  void rvalue(RValue*& rVal);
  void pval(Token& pVal);
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

void Parser::parse(Program& prog)
{
  advance();
  while (curr_token.type() != EOS) {
    if (curr_token.type() == TYPE){
      TypeDecl* tDec = new TypeDecl();
      tdecl(tDec);
      prog.decls.push_back(tDec);
    }
    else if(curr_token.type() == FUN){
      FunDecl* fDec = new FunDecl();
      fdecl(fDec);
      prog.decls.push_back(fDec);
    }
  }
  eat(EOS, "expecting end-of-file ");
}


void Parser::tdecl(TypeDecl*& tDec)
{
  advance();
  tDec->id = curr_token;
  eat(ID, "expecting variable ID ");
  vdecls(tDec->vdecls);
  eat(END, "expecting \'END\' keyword ");
}

void Parser::vdecls(std::list<VarDeclStmt*>& vdecs){
  while(curr_token.type() == VAR){
    VarDeclStmt* vDecl = new VarDeclStmt();
    vdecl_stmt(vDecl);
    vdecs.push_back(vDecl);
  }
}

void Parser::fdecl(FunDecl*& fDec)
{
  advance();
  if(curr_token.type() == NIL){
    fDec->return_type = curr_token;
    advance();
  }
  else dtype(fDec->return_type);
  fDec->id = curr_token;
  eat(ID, "expecting variable ID ");
  eat(LPAREN, "expecting '(' ");
  params(fDec->params);
  eat(RPAREN, "expecting ')' ");
  stmts(fDec->stmts);
  eat(END, "expecting 'END' keyword ");
}

void Parser::params(std::list<FunDecl::FunParam>& ps){
  if(curr_token.type() == ID){
    FunDecl::FunParam p;
    p.id = curr_token;
    advance();
    eat(COLON, "expecting ':' ");
    dtype(p.type);
    while(curr_token.type() == COMMA){
      advance();
      params(ps);
    }
  }
  else if(curr_token.type() != RPAREN)
    error("invalid parameter ");
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
  } else error("invalid declared type ");
}

void Parser::stmts(std::list<Stmt*>& stms){
  if(curr_token.type() == VAR ||
  curr_token.type() == ID ||
  curr_token.type() == IF ||
  curr_token.type() == WHILE ||
  curr_token.type() == FOR ||
  curr_token.type() == RETURN){
    stmt(stms);
    stmts(stms);
  }
}

void Parser::stmt(std::list<Stmt*>& stms){
  if(curr_token.type() == VAR){
    VarDeclStmt* vDecl =new VarDeclStmt();
    vdecl_stmt(vDecl);
    stms.push_back(vDecl);
  }
  else if(curr_token.type() == ID){
    Token id = curr_token;
    advance();
    if(curr_token.type() == LPAREN){
      CallExpr* cExpr = new CallExpr();
      cExpr->function_id = id;
      call_expr(cExpr);
      stms.push_back(cExpr);
    }
    else if(curr_token.type() == ASSIGN ||
            curr_token.type() == DOT){
      AssignStmt* aStmt = new AssignStmt();
      aStmt->lvalue_list.push_back(id);
      assign_stmt(aStmt);
      stms.push_back(aStmt);
    }
  }
  else if(curr_token.type() == IF){
    IfStmt* iStmt = new IfStmt();
    cond_stmt(iStmt);
    stms.push_back(iStmt);
  }
  else if(curr_token.type() == WHILE){
    WhileStmt* wStmt = new WhileStmt();
    while_stmt(wStmt);
    stms.push_back(wStmt);
  }
  else if(curr_token.type() == FOR){
    ForStmt* fStmt = new ForStmt();
    for_stmt(fStmt);
    stms.push_back(fStmt);
  }
  else if(curr_token.type() == RETURN){
    ReturnStmt* rStmt = new ReturnStmt();
    exit_stmt(rStmt);
    stms.push_back(rStmt);
  }
}

void Parser::vdecl_stmt(VarDeclStmt*& vDecl){
  advance();
  vDecl->id = curr_token;
  eat(ID, "expecting id ");
  if(curr_token.type() == COLON){
    advance();
    dtype(*vDecl->type);
  }
  eat(ASSIGN, "expecting '=' ");
  expr(vDecl->expr);
}

void Parser::assign_stmt(AssignStmt*& aStmt){
  // already added first id to the list (have to look for a dot not an id)
  lvalue(aStmt->lvalue_list);
  eat(ASSIGN, "expecting '=' ");
  expr(aStmt->expr);
}

void Parser::lvalue(std::list<Token>& lvalue_list){
  while(curr_token.type() == DOT){
    advance();
    lvalue_list.push_back(curr_token);
    eat(ID, "expecting id ");
  }
}

void Parser::cond_stmt(IfStmt*& iStmt){
  advance();
  BasicIf* ifPart = new BasicIf();
  expr(ifPart->expr);
  eat(THEN, "expecting 'then' keyword ");
  stmts(ifPart->stmts);
  iStmt->if_part = ifPart;
  condt(iStmt);
  eat(END, "expecting 'end' keyword ");
}

void Parser::condt(IfStmt*& iStmt){
  if(curr_token.type() == ELSEIF){
    advance();
    BasicIf* elseIf = new BasicIf();
    expr(elseIf->expr);
    eat(THEN, "expecting 'then' keyword ");
    stmts(elseIf->stmts);
    iStmt->else_ifs.push_back(elseIf);
    condt(iStmt);
  }
  else if(curr_token.type() == ELSE){
    advance();
    stmts(iStmt->body_stmts);
  }
}

void Parser::while_stmt(WhileStmt*& wStmt){
  advance();
  expr(wStmt->expr);
  eat(DO, "expecting 'do' keyword ");
  stmts(wStmt->stmts);
  eat(END, "expecting 'end' keyword ");
}

void Parser::for_stmt(ForStmt*& fStmt){
  advance();
  fStmt->var_id = curr_token;
  eat(ID, "expecting id ");
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

void Parser::args(std::list<Expr*>& arg_list){
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
    Expr* e = new Expr();
    expr(e);
    arg_list.push_back(e);
    while(curr_token.type() == COMMA){
      advance();
      args(arg_list);
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
  else {
    SimpleTerm* sTerm = new SimpleTerm();
    rvalue(sTerm->rvalue);
    exp->first = sTerm;
  }
  if(is_operator(curr_token.type())){
    op(exp->op);
    expr(exp->rest);
  }
}

void Parser::op(Token*& op){
  if(is_operator(curr_token.type())){
    *op = curr_token;
    advance();
  }
  else error("expecting operator ");
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
  else if(curr_token.type() == NIL){
    SimpleRValue* sRVal = new SimpleRValue();
    sRVal->value = curr_token;
    advance();
    rVal = sRVal;
  }
  else if(curr_token.type() == NEW){
    advance();
    NewRValue* nRVal = new NewRValue();
    nRVal->type_id = curr_token;
    eat(ID, "expecting type id ");
    rVal = nRVal;
  }
  else if(curr_token.type() == ID){
    Token id = curr_token;
    advance();
    if(curr_token.type() == LPAREN){
      CallExpr* cExpr = new CallExpr();
      cExpr->function_id = id;
      call_expr(cExpr);
      rVal = cExpr;
    }
    else {
      IDRValue* idrVal = new IDRValue();
      idrval(idrVal);
      rVal = idrVal;
    }
  }
  else if(curr_token.type() == NEG){
    advance();
    NegatedRValue* nRVal = new NegatedRValue();
    expr(nRVal->expr);
    rVal = nRVal;
  }  
}

void Parser::pval(Token& pVal){
  if(curr_token.type() == INT_VAL || 
    curr_token.type() == DOUBLE_VAL ||
    curr_token.type() == BOOL_VAL ||
    curr_token.type() == CHAR_VAL ||
    curr_token.type() == STRING_VAL){
    pVal = curr_token;
    advance();
  }
  else error("expecting value ");
}

void Parser::idrval(IDRValue*& idrVal){
  while(curr_token.type() == DOT){
    advance();
    idrVal->path.push_back(curr_token);
    eat(ID, "expecting id ");
  }
}

#endif
