//----------------------------------------------------------------------
// NAME:
// FILE:
// DATE:
// DESC:
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
  std::cout << "fun " << node.return_type.lexeme() << " " <<
               node.id.lexeme() << "(";
  inc_indent();
  for(FunDecl::FunParam param: node.params)
    std::cout << param.id.lexeme() << ": " << param.type.lexeme() <<
                 ", ";
  std::cout << ")" << std::endl;
  for(Stmt* stmt: node.stmts){
    std::cout << "   ";
    stmt->accept(*this);
  }
  std::cout << std::endl << "end" << std::endl << std::endl;
}

void Printer::visit(TypeDecl& node){
  std::cout << "type " << node.id.lexeme() << std::endl;
  for(VarDeclStmt* vDecl: node.vdecls){
    std::cout << "  ";
    vDecl->accept(*this);
  }
  std::cout << std::endl << std::endl;
}


//----------------------------------------------------------------------
// Statement Visitor Functions
//----------------------------------------------------------------------


void Printer::visit(VarDeclStmt& node){
  std::cout << "var" << node.id.lexeme();
  if(node.type)
    std::cout << ": " << node.type->lexeme();
  std::cout << " = ";
  node.expr->accept(*this);
}

void Printer::visit(AssignStmt& node){
  for(Token lVal: node.lvalue_list)
    std::cout << lVal.lexeme() << " = ";
  node.expr->accept(*this); 
  std::cout << std::endl; 
}

void Printer::visit(ReturnStmt& node){
  std::cout << "return ";
  node.expr->accept(*this);
  std::cout << std::endl;
}

void Printer::visit(IfStmt& node){
  std::cout << "if ";
  node.if_part->expr->accept(*this);
  std::cout << " then" << std::endl;
  for(Stmt* s: node.if_part->stmts){
    std::cout << "   ";
    s->accept(*this);
  }
  for(BasicIf* bIf: node.else_ifs){
    std::cout << std::endl << "elseif ";
    bIf->expr->accept(*this);
    std::cout << " then" << std::endl;
    for(Stmt* s: bIf->stmts){
      std::cout << "   ";
      s->accept(*this);
    }
  }
  if(node.body_stmts.size() > 0){
    std::cout << std::endl << "else" << std::endl;
    for(Stmt* s: node.body_stmts){
      std::cout << "   ";
      s->accept(*this);
    }
  }
  std::cout << std::endl << "end" << std::endl;
}

void Printer::visit(WhileStmt& node){
  std::cout << "while ";
  node.expr->accept(*this);
  std::cout << " do" <<std::endl;
  for(Stmt* s: node.stmts){
    std::cout << "   ";
    s->accept(*this);
  }
  std::cout << "end" << std::endl;
}

void Printer::visit(ForStmt& node){
  std::cout << "for " << node.var_id.lexeme() << "=";
  node.start->accept(*this);
  std::cout << " to ";
  node.end->accept(*this);
  std::cout << " do" <<std::endl;
  for(Stmt* s: node.stmts){
    std::cout << "   ";
    s->accept(*this);
  }
}


//----------------------------------------------------------------------
// Expression Visitor Functions
//----------------------------------------------------------------------


void Printer::visit(Expr& node){
  if(node.negated) std::cout << "not ";
  node.first->accept(*this);
  if(node.op) {
    std::cout << node.op->lexeme();
    node.rest->accept(*this);
  }
}

void Printer::visit(SimpleTerm& node){
  node.rvalue->accept(*this);
}

void Printer::visit(ComplexTerm& node){
  node.expr->accept(*this);
}


//----------------------------------------------------------------------
// RValue Visitor Functions
//----------------------------------------------------------------------


void Printer::visit(SimpleRValue& node){
  std::cout << node.value.lexeme();
}

void Printer::visit(NewRValue& node){
  std::cout << "new " << node.type_id.lexeme() << std::endl;
}

void Printer::visit(CallExpr& node){
  std::cout << node.function_id.lexeme() << "(";
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
  for(Token t: node.path){
    std::cout << t.lexeme();
  }
}

void Printer::visit(NegatedRValue& node){
  std::cout << "not ";
  node.expr->accept(*this);
  std::cout << std::endl;
}




#endif
