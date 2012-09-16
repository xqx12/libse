#include "gtest/gtest.h"
#include "interpreter.h"

using namespace se;

TEST(InterpreterTest, UnsupportedCharVar) {
  SpInterpreter sp_interpreter;

  EXPECT_THROW(sp_interpreter.visit(AnyExpr<char>("A")), InterpreterException);
}

TEST(InterpreterTest, UnsupportedCharValue) {
  SpInterpreter sp_interpreter;

  EXPECT_THROW(sp_interpreter.visit(ValueExpr<char>('A')), InterpreterException);
}

TEST(InterpreterTest, UnsupportedShortIntVar) {
  SpInterpreter sp_interpreter;

  EXPECT_THROW(sp_interpreter.visit(AnyExpr<short int>("A")), InterpreterException);
}

TEST(InterpreterTest, UnsupportedShortIntValue) {
  SpInterpreter sp_interpreter;

  EXPECT_THROW(sp_interpreter.visit(ValueExpr<short int>(5)), InterpreterException);
}

TEST(InterpreterTest, EqualityUnsat) {
  const SharedExpr a = SharedExpr(new AnyExpr<int>("A"));
  const SharedExpr b = SharedExpr(new AnyExpr<int>("A"));
  const SharedExpr equality = SharedExpr(new NaryExpr(EQL, OperatorInfo<EQL>::attr, a, b));
  const SharedExpr not_equality = SharedExpr(new UnaryExpr(NOT, equality));
  
  SpInterpreter sp_interpreter;
  sp_interpreter.solver.add(not_equality->walk(&sp_interpreter));

  EXPECT_EQ(z3::unsat, sp_interpreter.solver.check());
}

TEST(InterpreterTest, EqualitySat) {
  const SharedExpr a = SharedExpr(new AnyExpr<int>("A"));
  const SharedExpr b = SharedExpr(new AnyExpr<int>("B"));
  const SharedExpr equality = SharedExpr(new NaryExpr(EQL, OperatorInfo<EQL>::attr, a, b));
  const SharedExpr not_equality = SharedExpr(new UnaryExpr(NOT, equality));
  
  SpInterpreter sp_interpreter;
  sp_interpreter.solver.add(not_equality->walk(&sp_interpreter));

  EXPECT_EQ(z3::sat, sp_interpreter.solver.check());
}

TEST(InterpreterTest, SpInterpreterSatTernaryEquivalence) {
  const SharedExpr a = SharedExpr(new AnyExpr<int>("A"));
  const SharedExpr five = SharedExpr(new ValueExpr<int>(5));
  const SharedExpr lss = SharedExpr(new NaryExpr(LSS, OperatorInfo<LSS>::attr, a, five));
  const SharedExpr neg = SharedExpr(new UnaryExpr(NOT, lss));
  const SharedExpr c = SharedExpr(new AnyExpr<int>("C"));
  const SharedExpr d = SharedExpr(new AnyExpr<int>("D"));
  const SharedExpr ternary = SharedExpr(new IfThenElseExpr(neg, c, d));

  SpInterpreter sp_interpreter;
  z3::expr e = sp_interpreter.context.int_const("E");

  sp_interpreter.solver.add((e == ternary->walk(&sp_interpreter)) != 
                ((!(a->walk(&sp_interpreter) < 5) && e == c->walk(&sp_interpreter)) ||
                    ((a->walk(&sp_interpreter) < 5) && e == d->walk(&sp_interpreter))));

  // Proves that [e == ((!(a < 5)) ? c : d)] is equivalent to
  // [(!(a < 5) && e == c) || ((a < 5) && e == d)].
  EXPECT_EQ(z3::unsat, sp_interpreter.solver.check());
}

TEST(InterpreterTest, SpInterpreterUnsatTernaryEquivalence) {
  const SharedExpr a = SharedExpr(new AnyExpr<int>("A"));
  const SharedExpr five = SharedExpr(new ValueExpr<int>(5));
  const SharedExpr lss = SharedExpr(new NaryExpr(LSS, OperatorInfo<LSS>::attr, a, five));
  const SharedExpr neg = SharedExpr(new UnaryExpr(NOT, lss));
  const SharedExpr c = SharedExpr(new AnyExpr<int>("C"));
  const SharedExpr d = SharedExpr(new AnyExpr<int>("D"));
  const SharedExpr ternary = SharedExpr(new IfThenElseExpr(neg, c, d));

  SpInterpreter sp_interpreter;
  z3::expr e = sp_interpreter.context.int_const("E");

  sp_interpreter.solver.add((e == ternary->walk(&sp_interpreter)) != 
                ((!(a->walk(&sp_interpreter) < 5) && e == c->walk(&sp_interpreter)) ||
                    ((a->walk(&sp_interpreter) < 5) && e == (d->walk(&sp_interpreter) + 1))));

  // Disproves that [e == ((!(a < 5)) ? c : d)] is equivalent to
  // [(!(a < 5) && e == c) || ((a < 5) && e == (d + 1))].
  EXPECT_EQ(z3::sat, sp_interpreter.solver.check());
}

TEST(InterpreterTest, SpInterpreterWithSatNaryExprAsBinaryExpr) {
  const SharedExpr a = SharedExpr(new AnyExpr<int>("A"));
  const SharedExpr b = SharedExpr(new ValueExpr<int>(-2));
  const SharedExpr lss = SharedExpr(new NaryExpr(LSS, OperatorInfo<LSS>::attr, a, b));
  
  SpInterpreter sp_interpreter;
  sp_interpreter.solver.add(lss->walk(&sp_interpreter));

  sp_interpreter.solver.check();
  EXPECT_EQ(z3::sat, sp_interpreter.solver.check());

  std::stringstream out;
  out << sp_interpreter.solver.get_model();

  EXPECT_EQ("(define-fun A () Int\n  (- 3))", out.str());
}

TEST(InterpreterTest, SpInterpreterWithUnsatNaryExprAsBinaryExpr) {
  const SharedExpr a = SharedExpr(new AnyExpr<int>("A"));
  const SharedExpr b = SharedExpr(new ValueExpr<int>(5));
  const SharedExpr lss = SharedExpr(new NaryExpr(LSS, OperatorInfo<LSS>::attr, a, b));
  const SharedExpr neg = SharedExpr(new UnaryExpr(NOT, lss));
  
  SpInterpreter sp_interpreter;
  sp_interpreter.solver.add(lss->walk(&sp_interpreter));
  sp_interpreter.solver.add(neg->walk(&sp_interpreter));

  EXPECT_EQ(z3::unsat, sp_interpreter.solver.check());
}

TEST(InterpreterTest, SpInterpreterWithSatIfThenElseExpr) {
  const SharedExpr a = SharedExpr(new AnyExpr<int>("A"));
  const SharedExpr b = SharedExpr(new ValueExpr<int>(5));
  const SharedExpr lss = SharedExpr(new NaryExpr(LSS, OperatorInfo<LSS>::attr, a, b));
  const SharedExpr neg = SharedExpr(new UnaryExpr(NOT, lss));
  const SharedExpr taut = SharedExpr(new ValueExpr<bool>(true));
  const SharedExpr ternary = SharedExpr(new IfThenElseExpr(lss, neg, taut));
  
  SpInterpreter sp_interpreter;
  sp_interpreter.solver.add(ternary->walk(&sp_interpreter));

  EXPECT_EQ(z3::sat, sp_interpreter.solver.check());
}

TEST(InterpreterTest, SpInterpreterWithUnsatIfThenElseExpr) {
  const SharedExpr a = SharedExpr(new AnyExpr<int>("A"));
  const SharedExpr b = SharedExpr(new ValueExpr<int>(5));
  const SharedExpr lss = SharedExpr(new NaryExpr(LSS, OperatorInfo<LSS>::attr, a, b));
  const SharedExpr neg = SharedExpr(new UnaryExpr(NOT, lss));
  const SharedExpr falsum = SharedExpr(new ValueExpr<bool>(false));
  const SharedExpr ternary = SharedExpr(new IfThenElseExpr(lss, neg, falsum));
  
  SpInterpreter sp_interpreter;
  sp_interpreter.solver.add(ternary->walk(&sp_interpreter));

  EXPECT_EQ(z3::unsat, sp_interpreter.solver.check());
}

TEST(InterpreterTest, SpInterpreterWithTrueNaryExpr) {
  const SharedExpr true_expr = SharedExpr(new ValueExpr<bool>(true));
  NaryExpr true_nary = NaryExpr(LOR, OperatorInfo<LOR>::attr, true_expr);

  SpInterpreter sp_interpreter;
  EXPECT_NO_THROW(sp_interpreter.visit(true_nary));

  sp_interpreter.solver.add(true_nary.walk(&sp_interpreter));
  EXPECT_EQ(z3::sat, sp_interpreter.solver.check());
}

TEST(InterpreterTest, SpInterpreterWithFalseNaryExpr) {
  const SharedExpr false_expr = SharedExpr(new ValueExpr<bool>(false));
  NaryExpr false_nary = NaryExpr(LOR, OperatorInfo<LOR>::attr, false_expr);

  SpInterpreter sp_interpreter;
  EXPECT_NO_THROW(sp_interpreter.visit(false_nary));

  sp_interpreter.solver.add(false_nary.walk(&sp_interpreter));
  EXPECT_EQ(z3::unsat, sp_interpreter.solver.check());
}

TEST(InterpreterTest, SpInterpreterWithNaryExprThrows) {
  NaryExpr nary = NaryExpr(ADD, OperatorInfo<ADD>::attr);
  SpInterpreter sp_interpreter;
  EXPECT_THROW(sp_interpreter.visit(nary), InterpreterException);

  const SharedExpr a = SharedExpr(new AnyExpr<int>("A"));
  nary.append_operand(a);
  EXPECT_NO_THROW(sp_interpreter.visit(nary));
}

TEST(InterpreterTest, SpInterpreterWithUnsatNaryExpr) {
  const SharedExpr a = SharedExpr(new AnyExpr<int>("A"));
  const SharedExpr b = SharedExpr(new ValueExpr<int>(5));
  const SharedExpr c = SharedExpr(new ValueExpr<int>(3));
  NaryExpr nary = NaryExpr(ADD, OperatorInfo<ADD>::attr);
  nary.append_operand(a);
  nary.append_operand(b);
  nary.append_operand(c);

  SpInterpreter sp_interpreter;
  sp_interpreter.solver.add(sp_interpreter.visit(nary) != (a->walk(&sp_interpreter) + 8));

  EXPECT_EQ(z3::unsat, sp_interpreter.solver.check());
}

TEST(InterpreterTest, SpInterpreterWithSatNaryExpr) {
  const SharedExpr a = SharedExpr(new AnyExpr<int>("A"));
  const SharedExpr b = SharedExpr(new ValueExpr<int>(5));
  const SharedExpr c = SharedExpr(new ValueExpr<int>(3));
  NaryExpr nary = NaryExpr(ADD, OperatorInfo<ADD>::attr);
  nary.append_operand(a);
  nary.append_operand(b);
  nary.append_operand(c);

  SpInterpreter sp_interpreter;
  sp_interpreter.solver.add(sp_interpreter.visit(nary) == 12);

  EXPECT_EQ(z3::sat, sp_interpreter.solver.check());

  std::stringstream out;
  out << sp_interpreter.solver.get_model();

  EXPECT_EQ("(define-fun A () Int\n  4)", out.str());
}

