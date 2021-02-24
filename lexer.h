//----------------------------------------------------------------------
// NAME: Joshua Seward
// FILE: lexer.h
// DATE: 1/31/2020
// DESC: Lexer for MyPL.
//----------------------------------------------------------------------

#ifndef LEXER_H
#define LEXER_H

#include <istream>
#include <string>
#include "token.h"
#include "mypl_exception.h"


class Lexer
{
public:

  // construct a new lexer from the input stream
  Lexer(std::istream& input_stream);

  // return the next available token in the input stream (including
  // EOS if at the end of the stream)
  Token next_token();
  
private:

  // input stream, current line, and current column
  std::istream& input_stream;
  int line;
  int column;

  // return a single character from the input stream and advance
  char read();

  // return a single character from the input stream without advancing
  char peek();

  // create and throw a mypl_exception (exits the lexer)
  void error(const std::string& msg, int line, int column) const;
};


Lexer::Lexer(std::istream& input_stream)
  : input_stream(input_stream), line(1), column(1)
{
}


char Lexer::read()
{
  column++;
  return input_stream.get();
}


char Lexer::peek()
{
  return input_stream.peek();
}


void Lexer::error(const std::string& msg, int line, int column) const
{
  throw MyPLException(LEXER, msg, line, column);
}


Token Lexer::next_token()
{
  // Read through whitespace and comments
  
  while(std::isspace(peek()) || peek() == '#'){
    if(peek() == '#'){
    read();
    bool comment = true;
    while(comment){
      while(peek() != '\n'){
        read();
      }
      line++;
      column = 0;
      read();
      if(peek() != '#'){
        comment = false;
      }
    }
  }
  else if(peek() == '\n'){
    read();
    line++;
    column = 0;
  }
  else{
    read();
    }
  }

  // Check for EOF
  if(peek() == EOF){
    read();
    return Token(EOS, "", line, column-1);
  }


  // Check for simple symbols
  else if(peek() == ','){
    read();
    return Token(COMMA, ",", line, column-1);
  }
  else if(peek() == '('){
    read();
    return Token(LPAREN, "(", line, column-1);
  }
  else if(peek() == ')'){
    read();
    return Token(RPAREN, ")", line, column-1);
  }
  else if(peek() == ':'){
    read();
    return Token(COLON, ":", line, column-1);
  }
  else if(peek() == '+'){
    read();
    return Token(PLUS, "+", line, column-1);
  }
  else if(peek() == '-'){
    read();
    return Token(MINUS, "-", line, column-1);
  }
  else if(peek() == '*'){
    read();
    return Token(MULTIPLY, "*", line, column-1);
  }
  else if(peek() == '/'){
    read();
    return Token(DIVIDE, "/", line, column-1);
  }
  else if(peek() == '%'){
    read();
    return Token(MODULO, "%", line, column-1);
  }
  else if(peek() == '.'){
    read();
    return Token(DOT, ".", line, column-1);
  }


  // Check for more complicated symbols
  else if(peek() == '='){
    read();
    char nn_Token = peek();
    if(nn_Token == '='){
      read();
      return Token(EQUAL, "==", line, column-2);
    }
    return Token(ASSIGN, "=", line, column-1);
  }
  else if(peek() == '<'){
    read();
    char nn_Token = peek();
    if(nn_Token == '='){
      read();
      return Token(LESS_EQUAL, "<=", line, column-2);
    }
    return Token(LESS, "<", line, column-1);
  }
  else if(peek() == '>'){
    read();
    char nn_Token = peek();
    if(nn_Token == '='){
      read();
      return Token(GREATER_EQUAL, ">=", line, column-2);
    }
    return Token(GREATER, ">", line, column-1);
  }
  else if(peek() == '!'){
    read();
    char nn_Token = peek();
    if(nn_Token == '='){
      read();
      return Token(NOT_EQUAL, "!=", line, column-2);
    }
    error("! is invalid syntax", line, column);
  }
  else if(peek() == '.'){
    read();
    if(std::isspace(peek())) return Token(DOT, ".", line, column-1);
    else if(std::isdigit(peek()))
      error("Invalid Double - must lead with a numerical value", line, column);
  }


  // Check for char values
  else if(peek() == '\''){
    read();
    std::string lexeme = "";
    if(peek() == '\'') error("\'\' is an invalid char", line, column);
    else{
      lexeme += read();
      if(peek() == '\''){
        read();
        return Token(CHAR_VAL, lexeme, line, column-3);
      }
      else if(peek() != '\'') error("Expecting \'", line, column);
    }
  }


  // Check for strings
  else if(peek() == '\"'){
    read();
    std::string s = "";
    while(peek() != '\n' && peek() != '\"'){
      // special case for \"
      if(peek() == '\\'){
        s += read();
        if(peek() == '\"') s += read();
      }
      s += read();
    }
    if(peek() == '\n'){
      read();
      error("Expecting \"", line, column);
    }
    else if(peek() == '\"'){
      read();
      return Token(STRING_VAL, s, line, column-s.size()-2);
    }
  }


  // Check for numbers
  else if(std::isdigit(peek())){
    std::string lexeme = "";
    lexeme += read();
    while(!std::isspace(peek()) && peek()!='.' && std::isdigit(peek())){
      lexeme += read();
    }
    if(peek() == '.'){
      lexeme += read();
      while(!std::isspace(peek()) && std::isdigit(peek())){
        lexeme += read();
      }
      if(!std::isdigit(peek()) && !std::isspace(peek()))
        error("Invalid double value", line, column);
      else return Token(DOUBLE_VAL, lexeme, line, column-lexeme.size());
    }
    else if(!std::isalpha(peek())){
      return Token(INT_VAL, lexeme, line, column-lexeme.size());
    }
  }


  // Check for reserved words/ids
  else if(std::isalpha(peek())){
    std::string lexeme = "";
    lexeme += read();
    while(std::isalpha(peek()) || peek() == '_' || std::isdigit(peek())){
      lexeme += read();
    }
    if(lexeme == "neg"){
      return Token(NEG, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "and"){
      return Token(AND, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "or"){
      return Token(OR, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "not"){
      return Token(NOT, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "type"){
      return Token(TYPE, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "while"){
      return Token(WHILE, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "for"){
      return Token(FOR, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "to"){
      return Token(TO, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "do"){
      return Token(DO, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "if"){
      return Token(IF, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "then"){
      return Token(THEN, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "elseif"){
      return Token(ELSEIF, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "else"){
      return Token(ELSE, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "end"){
      return Token(END, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "fun"){
      return Token(FUN, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "var"){
      return Token(VAR, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "return"){
      return Token(RETURN, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "new"){
      return Token(NEW, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "bool"){
      return Token(BOOL_TYPE, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "int"){
      return Token(INT_TYPE, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "double"){
      return Token(DOUBLE_TYPE, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "char"){
      return Token(CHAR_TYPE, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "string"){
      return Token(STRING_TYPE, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "nil"){
      return Token(NIL, lexeme, line, column-lexeme.size());
    }
    else if(lexeme == "true" || lexeme == "false"){
      return Token(BOOL_VAL, lexeme, line, column-lexeme.size());
    }
    else{
      return Token(ID, lexeme, line, column-lexeme.size());
    } 
  }
  error("Unknown token " + std::string(peek(),1), line, column);
}


#endif
