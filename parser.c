/*
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 2.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reader.h"
#include "scanner.h"
#include "parser.h"
#include "error.h"

Token *currentToken;
Token *lookAhead;

int currentLine = 1;
int currentCol = 1;

void scan(void)
{
  Token *tmp = currentToken;
  currentToken = lookAhead;
  lookAhead = getValidToken();
  free(tmp);
}

void printLexeme(Token *token)
{
  if (token == NULL)
    return;

  while (currentLine < token->lineNo)
  {
    printf("\n");
    currentLine++;
    currentCol = 1;
  }

  while (currentCol < token->colNo)
  {
    printf(" ");
    currentCol++;
  }

  switch (token->tokenType)
  {
  case TK_IDENT:
  case TK_NUMBER:
    printf("%s", token->string);
    currentCol += strlen(token->string);
    break;
  case TK_CHAR:
    currentCol++;
    printf("\'%s\'", token->string);
    currentCol = currentCol + strlen(token->string) + 1;
    break;

  case SB_SEMICOLON:
    printf(";");
    currentCol++;
    break;

  case SB_COLON:
    printf(":");
    currentCol++;
    break;

  case SB_COMMA:
    printf(",");
    currentCol++;
    break;

  case SB_PERIOD:
    printf(".");
    currentCol++;
    break;

  case SB_ASSIGN:
    printf(":=");
    currentCol += 2;
    break;

  case SB_EQ:
    printf("=");
    currentCol++;
    break;

  case SB_GE:
    printf(">=");
    currentCol += 2;
    break;

  case SB_LE:
    printf("<=");
    currentCol += 2;
    break;

  case SB_LT:
    printf("<");
    currentCol++;
    break;

  case SB_GT:
    printf(">");
    currentCol++;
    break;

  case SB_NEQ:
    printf("!=");
    currentCol += 2;
    break;

  case SB_TIMES:
    printf("*");
    currentCol++;
    break;

  case SB_SLASH:
    printf("/");
    currentCol++;
    break;

  case SB_PLUS:
    printf("+");
    currentCol++;
    break;

  case SB_MINUS:
    printf("-");
    currentCol++;
    break;

  case SB_LPAR:
    printf("(");
    currentCol++;
    break;

  case SB_RPAR:
    printf(")");
    currentCol++;
    break;

  case SB_LSEL:
    printf("(.");
    currentCol += 2;
    break;

  case SB_RSEL:
    printf(".)");
    currentCol += 2;
    break;

  default:
    printf("%s", token->string);
    currentCol += strlen(token->string);
    break;
  }
}

void printError(ErrorCode err, int lineNo, int colNo)
{
  printf("\n");
  currentLine++;
  currentCol = 1;

  while (currentCol < colNo)
  {
    printf(" ");
    currentCol++;
  }

  printf("^\n");
  currentCol = 1;
}

void printMissing(TokenType tokenType, int lineNo, int colNo)
{
  printf("\n");
  currentLine++;
  currentCol = 1;

  while (currentCol < colNo)
  {
    printf(" ");
    currentCol++;
  }

  printf("^\n");
  currentCol = 1;
}

void eat(TokenType tokenType)
{
  if (lookAhead->tokenType == tokenType)
  {
    // printToken(lookAhead);
    printLexeme(lookAhead);
    scan();
  }
  else
  {
    printMissing(tokenType, lookAhead->lineNo, lookAhead->colNo);
    missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileProgram(void)
{
  // assert("Parsing a Program ....");
  eat(KW_PROGRAM);
  eat(TK_IDENT);
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_PERIOD);
  // assert("Program parsed!");
}

void compileBlock(void)
{
  // assert("Parsing a Block ....");
  if (lookAhead->tokenType == KW_CONST)
  {
    eat(KW_CONST);
    compileConstDecl();
    compileConstDecls();
    compileBlock2();
  }
  else
    compileBlock2();
  // assert("Block parsed!");
}

void compileBlock2(void)
{
  if (lookAhead->tokenType == KW_TYPE)
  {
    eat(KW_TYPE);
    compileTypeDecl();
    compileTypeDecls();
    compileBlock3();
  }
  else
    compileBlock3();
}

void compileBlock3(void)
{
  if (lookAhead->tokenType == KW_VAR)
  {
    eat(KW_VAR);
    compileVarDecl();
    compileVarDecls();
    compileBlock4();
  }
  else
  {
    compileBlock4();
  }
}

void compileBlock4(void)
{
  // assert("Parsing subtoutines ....");
  compileSubDecls();
  // assert("Subtoutines parsed ....");
  compileBlock5();
}

void compileBlock5(void)
{
  // assert("Parsing a group statement ....");
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
  // assert("Group statement parsed ....");
}

void compileConstDecls(void)
{
  while (lookAhead->tokenType == TK_IDENT)
  {
    compileConstDecl();
  }
}

void compileConstDecl(void)
{
  eat(TK_IDENT);
  eat(SB_EQ);
  compileConstant();
  eat(SB_SEMICOLON);
}

void compileTypeDecls(void)
{
  while (lookAhead->tokenType == TK_IDENT)
  {
    compileTypeDecl();
  }
}

void compileTypeDecl(void)
{
  eat(TK_IDENT);
  eat(SB_EQ);
  compileType();
  eat(SB_SEMICOLON);
}

void compileVarDecls(void)
{
  while (lookAhead->tokenType == TK_IDENT)
  {
    compileVarDecl();
  }
}

void compileVarDecl(void)
{
  eat(TK_IDENT);
  eat(SB_COLON);
  compileType();
  eat(SB_SEMICOLON);
}

void compileSubDecls(void)
{
  // assert("Parsing subtoutines ....");
  while (lookAhead->tokenType == KW_FUNCTION || lookAhead->tokenType == KW_PROCEDURE)
  {
    if (lookAhead->tokenType == KW_FUNCTION)
    {
      compileFuncDecl();
    }
    else
    {
      compileProcDecl();
    }
  }
  // assert("Subtoutines parsed ....");
}

void compileFuncDecl(void)
{
  // assert("Parsing a function ....");
  eat(KW_FUNCTION);
  eat(TK_IDENT);
  compileParams();
  eat(SB_COLON);
  compileBasicType();
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);
  // assert("Function parsed ....");
}

void compileProcDecl(void)
{
  // assert("Parsing a procedure ....");
  eat(KW_PROCEDURE);
  eat(TK_IDENT);
  compileParams();
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);
  // assert("Procedure parsed ....");
}

void compileUnsignedConstant(void)
{
  switch (lookAhead->tokenType)
  {
  case TK_NUMBER:
    eat(TK_NUMBER);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    break;
  default:
    printError(ERR_INVALIDCONSTANT, lookAhead->lineNo, lookAhead->colNo);
    error(ERR_INVALIDCONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileConstant(void)
{
  if (lookAhead->tokenType == SB_PLUS || lookAhead->tokenType == SB_MINUS)
  {
    scan();
    compileUnsignedConstant();
  }
  else
  {
    compileUnsignedConstant();
  }
}

void compileConstant2(void)
{
  switch (lookAhead->tokenType)
  {
  case TK_IDENT:
    eat(TK_IDENT);
    break;
  case TK_NUMBER:
    eat(TK_NUMBER);
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    break;
  default:
    printError(ERR_INVALIDCONSTANT, lookAhead->lineNo, lookAhead->colNo);
    error(ERR_INVALIDCONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileType(void)
{
  switch (lookAhead->tokenType)
  {
  case KW_INTEGER:
    compileBasicType();
    break;
  case KW_CHAR:
    compileBasicType();
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    break;
  case KW_ARRAY:
    eat(KW_ARRAY);
    eat(SB_LSEL);
    eat(TK_NUMBER);
    eat(SB_RSEL);
    eat(KW_OF);
    compileType();
    break;
  default:
    printError(ERR_INVALIDTYPE, lookAhead->lineNo, lookAhead->colNo);
    error(ERR_INVALIDTYPE, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileBasicType(void)
{
  if (lookAhead->tokenType == KW_INTEGER)
  {
    eat(KW_INTEGER);
  }
  else if (lookAhead->tokenType == KW_CHAR)
  {
    eat(KW_CHAR);
  }
  else
  {
    printError(ERR_INVALIDBASICTYPE, lookAhead->lineNo, lookAhead->colNo);
    error(ERR_INVALIDBASICTYPE, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileParams(void)
{
  if (lookAhead->tokenType == SB_LPAR)
  {
    eat(SB_LPAR);
    compileParam();
    compileParams2();
    eat(SB_RPAR);
  }
}

void compileParams2(void)
{
  while (lookAhead->tokenType == SB_SEMICOLON)
  {
    eat(SB_SEMICOLON);
    compileParam();
  }
}

void compileParam(void)
{
  if (lookAhead->tokenType == KW_VAR)
  {
    eat(KW_VAR);
  }
  eat(TK_IDENT);
  eat(SB_COLON);
  compileBasicType();
}

void compileStatements(void)
{
  compileStatement();
  compileStatements2();
}

void compileStatements2(void)
{
  while (lookAhead->tokenType == SB_SEMICOLON)
  {
    eat(SB_SEMICOLON);
    compileStatement();
  }
}

void compileStatement(void)
{

  switch (lookAhead->tokenType)
  {
  case TK_IDENT:
    compileAssignSt();
    break;
  case KW_CALL:
    compileCallSt();
    break;
  case KW_BEGIN:
    // assert("Parsing a group statement ....");
    compileBlock5();
    // assert("Group statement parsed ....");
    break;
  case KW_IF:
    compileIfSt();
    break;
  case KW_WHILE:
    compileWhileSt();
    break;
  case KW_FOR:
    compileForSt();
    break;
    // EmptySt needs to check FOLLOW tokens
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
    break;
    // Error occurs
  default:
    printError(ERR_INVALIDSTATEMENT, lookAhead->lineNo, lookAhead->colNo);
    error(ERR_INVALIDSTATEMENT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileAssignSt(void)
{
  // assert("Parsing an assign statement ....");
  eat(TK_IDENT);
  compileIndexes();
  eat(SB_ASSIGN);
  compileExpression();
  // assert("Assign statement parsed ....");
}

void compileCallSt(void)
{
  // assert("Parsing a call statement ....");
  eat(KW_CALL);
  eat(TK_IDENT);
  compileArguments();
  // assert("Call statement parsed ....");
}

void compileGroupSt(void)
{
  // assert("Parsing a group statement ....");
  eat(KW_WHILE);
  compileCondition();
  eat(KW_DO);
  compileStatement();
  // assert("Group statement parsed ....");
}

void compileIfSt(void)
{
  // assert("Parsing an if statement ....");
  eat(KW_IF);
  compileCondition();
  eat(KW_THEN);
  compileStatement();
  if (lookAhead->tokenType == KW_ELSE)
    compileElseSt();
  // assert("If statement parsed ....");
}

void compileElseSt(void)
{
  eat(KW_ELSE);
  compileStatement();
}

void compileWhileSt(void)
{
  // assert("Parsing a while statement ....");
  eat(KW_WHILE);
  compileCondition();
  eat(KW_DO);
  compileStatement();
  // assert("While statement parsed ....");
}

void compileForSt(void)
{
  // assert("Parsing a for statement ....");
  eat(KW_FOR);
  eat(TK_IDENT);
  eat(SB_ASSIGN);
  compileExpression();
  eat(KW_TO);
  compileExpression();
  eat(KW_DO);
  compileStatement();
  // assert("For statement parsed ....");
}

void compileArguments(void)
{
  if (lookAhead->tokenType == SB_LPAR)
  {
    eat(SB_LPAR);
    compileExpression();
    compileArguments2();
    eat(SB_RPAR);
  }
}

void compileArguments2(void)
{
  while (lookAhead->tokenType == SB_COMMA)
  {
    eat(SB_COMMA);
    compileExpression();
  }
}

void compileCondition(void)
{
  compileExpression();
  compileCondition2();
}

void compileCondition2(void)
{
  switch (lookAhead->tokenType)
  {
  case SB_EQ:
    eat(SB_EQ);
    compileExpression();
    break;
  case SB_NEQ:
    eat(SB_NEQ);
    compileExpression();
    break;
  case SB_LT:
    eat(SB_LT);
    compileExpression();
    break;
  case SB_LE:
    eat(SB_LE);
    compileExpression();
    break;
  case SB_GT:
    eat(SB_GT);
    compileExpression();
    break;
  case SB_GE:
    eat(SB_GE);
    scan();
    compileExpression();
    break;
  default:
    printError(ERR_INVALIDCONSTANT, lookAhead->lineNo, lookAhead->colNo);
    error(ERR_INVALIDCONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileExpression(void)
{
  // assert("Parsing an expression");
  if (lookAhead->tokenType == SB_PLUS || lookAhead->tokenType == SB_MINUS)
  {
    scan();
  }
  compileExpression2();
  // assert("Expression parsed");
}

void compileExpression2(void)
{
  compileTerm();
  compileExpression3();
}

void compileExpression3(void)
{
  while (lookAhead->tokenType == SB_PLUS || lookAhead->tokenType == SB_MINUS)
  {
    eat(lookAhead->tokenType);
    compileTerm();
  }
}

void compileTerm(void)
{
  compileFactor();
  compileTerm2();
}

void compileTerm2(void)
{
  while (lookAhead->tokenType == SB_TIMES || lookAhead->tokenType == SB_SLASH)
  {
    eat(lookAhead->tokenType);
    compileFactor();
  }
}

void compileFactor(void)
{
  switch (lookAhead->tokenType)
  {
  case TK_NUMBER:
    eat(TK_NUMBER);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    if (lookAhead->tokenType == SB_LSEL)
    {
      compileIndexes();
    }
    else if (lookAhead->tokenType == SB_LPAR)
    {
      compileArguments();
    }
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    break;
  case SB_LPAR:
    eat(SB_LPAR);
    compileExpression();
    eat(SB_RPAR);
    break;
  default:
    printError(ERR_INVALIDFACTOR, lookAhead->lineNo, lookAhead->colNo);
    error(ERR_INVALIDFACTOR, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileIndexes(void)
{
  while (lookAhead->tokenType == SB_LSEL)
  {
    eat(SB_LSEL);
    compileExpression();
    eat(SB_RSEL);
  }
}

int compile(char *fileName)
{
  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  currentToken = NULL;
  lookAhead = getValidToken();

  compileProgram();

  free(currentToken);
  free(lookAhead);
  closeInputStream();
  return IO_SUCCESS;
}
