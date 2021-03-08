//----------------------------------------------------------------------
// NAME: Joshua Seward
// FILE: printer.h
// DATE: 2/27/2021
// DESC: Visitor functions for the ast.h file that "pretty print" the 
//       code being analyzed
//----------------------------------------------------------------------

#ifndef PRINTER_H
#define PRINTER_H

#include <iostream>
#include "ast.h"


class Printer : public Visitor
{
public:
  // constructor
  Printer(std::ostream& output_stream) : out(output_stream) {}

  // top-level
  void visit(Program& node);
  void visit(FunDecl& node);
  void visit(TypeDecl& node);
  // statements
  void visit(VarDeclStmt& node);
  void visit(AssignStmt& node);
  void visit(ReturnStmt& node);
  void visit(IfStmt& node);
  void visit(WhileStmt& node);
  void visit(ForStmt& node);
  // expressions
  void visit(Expr& node);
  void visit(SimpleTerm& node);
  void visit(ComplexTerm& node);
  // rvalues
  void visit(SimpleRValue& node);
  void visit(NewRValue& node);
  void visit(CallExpr& node);
  void visit(IDRValue& node);
  void visit(NegatedRValue& node);

private:
  std::ostream& out;
  int indent = 0;

  void inc_indent() {indent += 3;}
  void dec_indent() {indent -= 3;}
  std::string get_indent() {return std::string(indent, ' ');}

};


//----------------------------------------------------------------------
// Top-Level Visitor Functions
//----------------------------------------------------------------------


void Printer::visit(Program& node){
  for(Decl* d : node.decls)
    d->accept(*this);
}

void Printer::visit(FunDecl& node){
  // print out function header
  std::cout << std::endl << get_indent() << "fun " << node.return_type.lexeme() << " " <<
               node.id.lexeme() << "(";
  // print out parameters
  for(FunDecl::FunParam param: node.params){
    if(param.id.lexeme() == node.params.back().id.lexeme())
      std::cout << param.id.lexeme() << ": " << param.type.lexeme();
    else
      std::cout << param.id.lexeme() << ": " << param.type.lexeme() <<
                 ", ";
  }
  std::cout << ")" << std::endl;
  // print out function body
  inc_indent();
  for(Stmt* stmt: node.stmts){
    std::cout << get_indent();
    stmt->accept(*this);
    std::cout << std::endl;
  }
  // print out end keyword
  dec_indent();
  std::cout << get_indent() << "end" << std::endl;
}

void Printer::visit(TypeDecl& node){
  // print out type header
  std::cout << std::endl << get_indent() << "type " << node.id.lexeme() << std::endl;
  // print out type body
  for(VarDeclStmt* vDecl: node.vdecls){
    std::cout << "  " << get_indent();
    vDecl->accept(*this);
    std::cout << std::endl;
  }
  // print out end keyword
  std::cout << get_indent() << "end" << std::endl;
}


//----------------------------------------------------------------------
// Statement Visitor Functions
//----------------------------------------------------------------------


void Printer::visit(VarDeclStmt& node){
  // print out lhs
  std::cout << "var " << node.id.lexeme();
  // print out type (if necessary)
  if(node.type)
    std::cout << ": " << node.type->lexeme();
  // print out assignment op and rhs
  std::cout << " = ";
  node.expr->accept(*this);
}

void Printer::visit(AssignStmt& node){
  // print out lhs path
  auto lhs = node.lvalue_list.begin();
  for(int i = 0; i < node.lvalue_list.size()-1; ++i){
    std::advance(lhs, i);
    Token t = *lhs;
    std::cout << t.lexeme() << ".";
  }
  // print out assignment operator and rhs expression
  std::cout << node.lvalue_list.back().lexeme() << " = ";
  node.expr->accept(*this); 
}

void Printer::visit(ReturnStmt& node){
  // print out return statement
  std::cout << "return ";
  node.expr->accept(*this);
}

void Printer::visit(IfStmt& node){
  // print out if statement header
  std::cout << "if ";
  node.if_part->expr->accept(*this);
  std::cout << " then" << std::endl;
  // print out if statement body
  inc_indent();
  for(Stmt* s: node.if_part->stmts){
    std::cout << get_indent();
    s->accept(*this);
    std::cout << std::endl;
  }
  dec_indent();
  // print out elseif statements (if there are any)
  for(BasicIf* bIf: node.else_ifs){
    std::cout << get_indent() << "elseif ";
    bIf->expr->accept(*this);
    std::cout << " then" << std::endl;
    inc_indent();
    for(Stmt* s: bIf->stmts){
      std::cout << get_indent();
      s->accept(*this);
      std::cout << std::endl;
    }
    dec_indent();
  }
  // print out else statement (if it exists)
  if(node.body_stmts.size() > 0){
    std::cout << get_indent() << "else" << std::endl;
    inc_indent();
    for(Stmt* s: node.body_stmts){
      std::cout << get_indent();
      s->accept(*this);
      std::cout << std::endl;
    }
    dec_indent();
  }
  std::cout << get_indent() << "end";
}

void Printer::visit(WhileStmt& node){
  // print out while statement header
  std::cout << "while ";
  node.expr->accept(*this);
  std::cout << " do " << std::endl;
  // print out while statement body
  inc_indent();
  for(Stmt* s: node.stmts){
    std::cout << get_indent();
    s->accept(*this);
    std::cout << std::endl;
  }
  dec_indent();
  std::cout << get_indent() << "end";
}

void Printer::visit(ForStmt& node){
  // print out for statement header
  std::cout << "for " << node.var_id.lexeme() << "=";
  node.start->accept(*this);
  std::cout << " to ";
  node.end->accept(*this);
  std::cout << " do " <<std::endl;
  // print out for statement body
  inc_indent();
  for(Stmt* s: node.stmts){
    std::cout << get_indent();
    s->accept(*this);
    std::cout << std::endl;
  }
  dec_indent();
  std::cout << get_indent() << "end";
}


//----------------------------------------------------------------------
// Expression Visitor Functions
//----------------------------------------------------------------------


void Printer::visit(Expr& node){
  // print out expression
  if(node.negated){
    std::cout << "not ";
    node.first->accept(*this);
  }
  if(node.op) {
    node.first->accept(*this);
    std::cout << " " << node.op->lexeme() << " ";
    node.rest->accept(*this);
  }
  else node.first->accept(*this);
}

void Printer::visit(SimpleTerm& node){
  node.rvalue->accept(*this);
}

void Printer::visit(ComplexTerm& node){
  std::cout << "(";
  node.expr->accept(*this);
  std::cout << ")";
}


//----------------------------------------------------------------------
// RValue Visitor Functions
//----------------------------------------------------------------------


void Printer::visit(SimpleRValue& node){
  if(node.value.type() == STRING_VAL)
    std::cout << "\"" << node.value.lexeme() << "\"";
  else if(node.value.type() == CHAR_VAL)
    std::cout << "'" << node.value.lexeme() << "'";
  else std::cout << node.value.lexeme();
}

void Printer::visit(NewRValue& node){
  std::cout << "new " << node.type_id.lexeme();
}

void Printer::visit(CallExpr& node){
  // print out the function id
  std::cout << node.function_id.lexeme() << "(";
  // print out the arguments being passed into the function
  if(node.arg_list.size() > 0){
    for(int i = 0; i < node.arg_list.size()-1; ++i){
      auto it = node.arg_list.begin();
      std::advance(it, i);
      Expr* e = *it;
      e->accept(*this);
      std::cout << ", ";
    }
    auto it = node.arg_list.begin();
    std::advance(it, node.arg_list.size()-1);
    Expr* e = *it;
    e->accept(*this);
  }
  std::cout << ")";
}

void Printer::visit(IDRValue& node){
  auto id = node.path.begin();
  for(int i = 0; i < node.path.size()-1; ++i){
    std::advance(id, i);
    Token t = *id;
    std::cout << t.lexeme() << ".";
  }
  std::cout << node.path.back().lexeme();
}

void Printer::visit(NegatedRValue& node){
  std::cout << "not ";
  node.expr->accept(*this);
}




#endif
