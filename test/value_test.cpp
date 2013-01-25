#include "gtest/gtest.h"
#include "sequential-se.h"

using namespace se;

TEST(ValueTest, TypeConversions) {
  Value<bool> a = Value<bool>(true);
  Value<bool> b = Value<bool>(false);
  Value<char> c = Value<char>('A');
  Value<int> d = Value<int>(42);

  EXPECT_TRUE(static_cast<bool>(a));
  EXPECT_FALSE(static_cast<bool>(b));
  EXPECT_EQ('A', static_cast<char>(c));
  EXPECT_EQ(42, static_cast<int>(d));

  // test constness conversions
  const Value<bool>& ca = Value<bool>(true);
  const Value<bool>& cb = Value<bool>(false);
  const Value<char>& cc = Value<char>('A');
  const Value<int>& cd = Value<int>(42);

  EXPECT_TRUE(static_cast<bool>(ca));
  EXPECT_FALSE(static_cast<bool>(cb));
  EXPECT_EQ('A', static_cast<char>(cc));
  EXPECT_EQ(42, static_cast<int>(cd));
}

TEST(ValueTest, BoolConversion) {
  EXPECT_TRUE(make_value<bool>(true));
  EXPECT_FALSE(make_value<bool>(false));
}

class BoolOperator {
private:
  const bool flag;
public:
  BoolOperator(const bool flag) : flag(flag) {}
  operator bool() const { return flag; }
};

TEST(ValueTest, Types) {
  const Value<bool> a = Value<bool>(true);
  const Value<char> b = Value<char>(3);
  const Value<int> c = Value<int>(5);

  EXPECT_EQ(BOOL, a.type());
  EXPECT_EQ(CHAR, b.type());
  EXPECT_EQ(INT, c.type());
}

TEST(ValueTest, ValueConstructorWithExpression) {
  const SharedExpr expr = SharedExpr(new ValueExpr<char>(5));
  const Value<char> a = Value<char>(5, expr);

  EXPECT_EQ(CHAR, a.type());
  EXPECT_EQ(5, a.data());
  EXPECT_TRUE(a.is_symbolic());
}

TEST(ValueTest, ValueConstructorWithoutExpression) {
  const Value<char> a = Value<char>(5);

  EXPECT_EQ(CHAR, a.type());
  EXPECT_EQ(5, a.data());
  EXPECT_FALSE(a.is_symbolic());
}

TEST(ValueTest, ValueConstructorWithNullExpression) {
  const Value<char> a = Value<char>(5, SharedExpr());

  EXPECT_EQ(CHAR, a.type());
  EXPECT_EQ(5, a.data());
  EXPECT_FALSE(a.is_symbolic());
}

TEST(ValueTest, ValueCopyConstructorWithoutExpression) {
  const Value<char> a = Value<char>(5);
  const Value<char> b = a;

  EXPECT_EQ(CHAR, b.type());
  EXPECT_EQ(5, b.data());
  EXPECT_FALSE(b.is_symbolic());
}

TEST(ValueTest, ValueCopyConstructorWithoutExpressionButCast) {
  const Value<char> a = Value<char>(5);
  const Value<int> b = a;

  EXPECT_EQ(INT, b.type());
  EXPECT_EQ(5, b.data());
  EXPECT_FALSE(b.is_symbolic());
}

TEST(ValueTest, ValueCopyConstructorWithExpression) {
  const SharedExpr expr = SharedExpr(new ValueExpr<char>(5));
  const Value<char> a = Value<char>(5, expr);
  const Value<char> b = a;

  EXPECT_EQ(CHAR, b.type());
  EXPECT_EQ(5, b.data());
  EXPECT_EQ(expr, b.expr());
  EXPECT_TRUE(b.is_symbolic());
}

TEST(ValueTest, ValueCopyConstructorWithExpressionAndCast) {
  const SharedExpr expr = SharedExpr(new ValueExpr<char>(5));
  const Value<char> a = Value<char>(5, expr);
  const Value<int> b = a;

  EXPECT_EQ(INT, b.type());
  EXPECT_EQ(5, b.data());
  EXPECT_NE(expr, b.expr());
  EXPECT_TRUE(b.is_symbolic());
}

TEST(ValueTest, ValueCopyConstructorWithNullExpression) {
  const Value<char> a = Value<char>(5, SharedExpr());
  const Value<char> b = a;

  EXPECT_EQ(CHAR, b.type());
  EXPECT_EQ(5, b.data());
  EXPECT_FALSE(b.is_symbolic());
}

TEST(ValueTest, ValueCopyConstructorWithNullExpressionAndCast) {
  const Value<char> a = Value<char>(5, SharedExpr());
  const Value<int> b = a;

  EXPECT_EQ(INT, b.type());
  EXPECT_EQ(5, b.data());
  EXPECT_FALSE(b.is_symbolic());
}

TEST(ValueTest, ValueAssignmentWithoutExpressionButCast) {
  const Value<int> a = Value<int>(5);
  Value<char> b = Value<char>(3);

  b = a;

  // Typing information is immutable
  EXPECT_EQ(CHAR, b.type());

  EXPECT_EQ(5, b.data());
  EXPECT_FALSE(b.is_symbolic());
}

TEST(ValueTest, ValueAssignmentWithExpressionAndCast) {
  const SharedExpr expr = SharedExpr(new ValueExpr<int>(5));
  const Value<int> a = Value<int>(5, expr);
  Value<char> b = Value<char>(3);

  b = a;

  // Typing information is immutable
  EXPECT_EQ(CHAR, b.type());

  EXPECT_EQ(5, b.data());
  EXPECT_NE(expr, b.expr());
  EXPECT_TRUE(b.is_symbolic());
}

TEST(ValueTest, ValueAssignmentWithExpressionAndWithoutCast) {
  const SharedExpr expr = SharedExpr(new ValueExpr<int>(5));
  const Value<int> a = Value<int>(5, expr);
  Value<int> b = Value<int>(3);

  b = a;

  EXPECT_EQ(INT, b.type());

  EXPECT_EQ(5, b.data());
  EXPECT_EQ(expr, b.expr());
  EXPECT_TRUE(b.is_symbolic());
}

TEST(ValueTest, ValueAssignmentWithNullExpressionAndCast) {
  const Value<int> a = Value<int>(5, SharedExpr());
  Value<char> b = Value<char>(3);

  b = a;

  // Typing information is immutable
  EXPECT_EQ(CHAR, b.type());

  EXPECT_EQ(5, b.data());
  EXPECT_FALSE(b.is_symbolic());
}

TEST(ValueTest, ValueAssignmentWithNullExpressionAndWithoutCast) {
  const Value<int> a = Value<int>(5, SharedExpr());
  Value<int> b = Value<int>(3);

  b = a;

  EXPECT_EQ(INT, b.type());
  EXPECT_EQ(5, b.data());
  EXPECT_FALSE(b.is_symbolic());
}

template<typename T>
const Value<T> reflect_with_expr(const T data) {
  Value<T> a = make_value<T>(data);
  a.set_expr(SharedExpr(new ValueExpr<T>(data)));
  return a;
}

TEST(ValueTest, AddWithoutExpression) {
  const Value<char>& a = make_value<char>(2);
  const Value<int>& b = make_value<int>(5);
  const AbstractValue& c = a + b;
  const AbstractValue& d = b + a;

  EXPECT_EQ(INT, c.type());
  EXPECT_EQ(INT, d.type());

  EXPECT_FALSE(c.is_symbolic());
  EXPECT_FALSE(d.is_symbolic());

  EXPECT_EQ(7, (a + b).data());
  EXPECT_EQ(7, (b + a).data());
}

TEST(ValueTest, AddWithExpression) {
  const Value<int>& a = reflect_with_expr<int>(2);
  const Value<int>& b = make_value<int>(5);
  const AbstractValue& c = a + b;
  const AbstractValue& d = b + a;

  EXPECT_EQ(INT, c.type());
  EXPECT_EQ(INT, d.type());

  EXPECT_TRUE(c.is_symbolic());
  EXPECT_TRUE(d.is_symbolic());

  EXPECT_EQ(7, (a + b).data());
  EXPECT_EQ(7, (b + a).data());

  std::stringstream out;
  out << c.expr();
  EXPECT_EQ("(2+5)", out.str());
}

TEST(ValueTest, LessThanWithoutExpression) {
  const Value<char>& a = make_value<char>(2);
  const Value<int>& b = make_value<int>(5);
  const AbstractValue& c = a < b;
  const AbstractValue& d = b < a;

  EXPECT_EQ(BOOL, c.type());
  EXPECT_EQ(BOOL, d.type());

  EXPECT_FALSE(c.is_symbolic());
  EXPECT_FALSE(d.is_symbolic());

  EXPECT_TRUE((a < b).data());
  EXPECT_FALSE((b < a).data());
}

TEST(ValueTest, LessThanWithExpression) {
  const Value<int>& a = reflect_with_expr<int>(2);
  const Value<int>& b = make_value<int>(5);
  const AbstractValue& c = a < b;
  const AbstractValue& d = b < a;

  EXPECT_EQ(BOOL, c.type());
  EXPECT_EQ(BOOL, d.type());

  EXPECT_TRUE(c.is_symbolic());
  EXPECT_TRUE(d.is_symbolic());

  EXPECT_TRUE((a < b).data());
  EXPECT_FALSE((b < a).data());

  std::stringstream out;
  out << c.expr();
  EXPECT_EQ("(2<5)", out.str());
}

TEST(ValueTest, NotTrueWithoutExpression) {
  const Value<bool>& a = make_value<bool>(true);
  const AbstractValue& b = !a;

  EXPECT_EQ(BOOL, b.type());
  EXPECT_FALSE(b.is_symbolic());
  EXPECT_FALSE((!a).data());
}

TEST(ValueTest, NotFalseWithoutExpression) {
  const Value<bool>& a = make_value<bool>(false);
  const AbstractValue& b = !a;

  EXPECT_EQ(BOOL, b.type());
  EXPECT_FALSE(b.is_symbolic());
  EXPECT_TRUE((!a).data());
}

TEST(ValueTest, NotTrueWithExpression) {
  const Value<bool>& a = reflect_with_expr<bool>(true);
  const AbstractValue& b = !a;

  EXPECT_EQ(BOOL, b.type());
  EXPECT_TRUE(b.is_symbolic());
  EXPECT_FALSE((!a).data());

  std::stringstream out;
  out << b.expr();
  EXPECT_EQ("(!1)", out.str());
}

TEST(ValueTest, NotFalseWithExpression) {
  const Value<bool>& a = reflect_with_expr<bool>(false);
  const AbstractValue& b = !a;

  EXPECT_EQ(BOOL, b.type());
  EXPECT_TRUE(b.is_symbolic());
  EXPECT_TRUE((!a).data());

  std::stringstream out;
  out << b.expr();
  EXPECT_EQ("(!0)", out.str());
}

TEST(ValueTest, SetSymbolic) {
  Value<int> a = make_value<int>(5);

  EXPECT_FALSE(a.is_symbolic());
  a.set_symbolic("a");
  EXPECT_TRUE(a.is_symbolic());
}

TEST(ValueTest, SetSymbolicName) {
  Value<int> a = make_value<int>(5);
  std::string name = "Var_0";

  a.set_symbolic(name);

  std::stringstream out;
  out << a.expr();
  EXPECT_EQ("[Var_0:5]", out.str());

  name.clear();
  std::stringstream after_clear;
  after_clear << a.expr();
  EXPECT_EQ("[Var_0:5]", after_clear.str());
}

// Reset a value's symbolic expression to a new ValueExpr<T> object.
TEST(ValueTest, SetSymbolicTwiceWithValue) {
  Value<int> a = make_value<int>(5);
  a.set_symbolic("A");

  std::stringstream out;
  out << a.expr();
  EXPECT_EQ("[A:5]", out.str());

  EXPECT_TRUE(a.is_symbolic());
  const SharedExpr& expr = a.expr();

  a.set_symbolic("__A");
  EXPECT_TRUE(a.is_symbolic());

  EXPECT_NE(expr, a.expr());
  
  std::stringstream __out;
  __out << a.expr();
  EXPECT_EQ("[__A:5]", __out.str());
}

// Reset a value's symbolic expression to a new AnyExpr<T> object.
TEST(ValueTest, SetSymbolicTwiceWithAny) {
  Value<int> a = any<int>("A");

  std::stringstream out;
  out << a.expr();
  EXPECT_EQ("[A]", out.str());

  EXPECT_TRUE(a.is_symbolic());
  const SharedExpr& expr = a.expr();

  a.set_symbolic("__A");
  EXPECT_TRUE(a.is_symbolic());

  EXPECT_NE(expr, a.expr());
  
  std::stringstream __out;
  __out << a.expr();
  EXPECT_EQ("[__A]", __out.str());
}

TEST(ValueTest, NativeBitPrecision) {
  // C++ standard does not specify if char is signed or unsigned
  char x = 254;
  int y = 5;
  int z = x + y;

  const Value<char>& a = make_value(x);
  const Value<int>& b = make_value(y);
  const Value<int>& c = a + b;

  // But C++ guarantees type promotion
  EXPECT_EQ(INT, (a + b).type());

  int v = c;
  EXPECT_EQ(z, v);
}

TEST(ValueTest, TypePromotion) {
  // C++ guarantees that a variable of type char is in the range [-128, 127].
  const Value<char>& a = make_value<char>(127);
  const Value<int>& b = make_value<int>(256);
  const AbstractValue& c = a + b;

  // C++ guarantees type promotion
  EXPECT_EQ(INT, c.type());

  int v = a + b;
  int w = b + a;
  EXPECT_EQ(383, v);
  EXPECT_EQ(383, w);
}

TEST(ValueTest, SymbolicBoolConversion) {
  tracer().reset();

  Value<bool> a = make_value<bool>(true);
  a.set_symbolic("A");

  Value<bool> b = make_value<bool>(false);
  b.set_symbolic("B");

  EXPECT_TRUE(a);
  EXPECT_FALSE(b);

  std::stringstream out;
  tracer().write_path_constraints(out);

  EXPECT_EQ("[A:1]\n(![B:0])\n", out.str());
}

TEST(ValueTest, SymbolicBoolConversionWithIfStatement) {
  tracer().reset();

  Value<bool> a = make_value<bool>(true);
  a.set_symbolic("A");

  Value<bool> b = make_value<bool>(false);
  b.set_symbolic("B");

  // C++ forces explicit conversion to bool
  if(a) {};
  if(b) {};

  std::stringstream out;
  tracer().write_path_constraints(out);

  EXPECT_EQ("[A:1]\n(![B:0])\n", out.str());
}

TEST(ValueTest, InitialAndSetAggregate) {
  Value<int> a(2);
  EXPECT_FALSE(a.has_aggregate());
  a.set_aggregate(3);
  EXPECT_TRUE(a.has_aggregate());

  Value<int> b(2, SharedExpr(new AnyExpr<int>("A")));
  EXPECT_FALSE(b.has_aggregate());
  b.set_aggregate(3);
  EXPECT_TRUE(b.has_aggregate());

  Value<int> c("A");
  EXPECT_FALSE(c.has_aggregate());
  c.set_aggregate(3);
  EXPECT_TRUE(c.has_aggregate());

  Value<int> d(a);
  EXPECT_TRUE(d.has_aggregate());
  EXPECT_EQ(3, d.aggregate());
  d.set_aggregate(4);
  EXPECT_TRUE(d.has_aggregate());

  Value<int> e(Value<int>(1));
  EXPECT_FALSE(e.has_aggregate());
  e.set_aggregate(3);
  EXPECT_TRUE(e.has_aggregate());
}

TEST(ValueTest, InitialAndSetSameAggregate) {
  Value<int> a(2);
  EXPECT_FALSE(a.has_aggregate());
  a.set_aggregate(2);
  EXPECT_TRUE(a.has_aggregate());

  Value<int> b(2, SharedExpr(new AnyExpr<int>("A")));
  EXPECT_FALSE(b.has_aggregate());
  b.set_aggregate(2);
  EXPECT_TRUE(b.has_aggregate());

  Value<int> d(a);
  EXPECT_TRUE(d.has_aggregate());
  EXPECT_EQ(2, d.aggregate());
  d.set_aggregate(2);
  EXPECT_TRUE(d.has_aggregate());

  Value<int> e(Value<int>(1));
  EXPECT_FALSE(e.has_aggregate());
  e.set_aggregate(1);
  EXPECT_TRUE(e.has_aggregate());
}

TEST(ValueTest, AssignmentOperatorWithAggregate) {
  Value<int> a(2);
  EXPECT_FALSE(a.has_aggregate());
  a.set_aggregate(2);
  EXPECT_TRUE(a.has_aggregate());

  Value<int> b(2, SharedExpr(new AnyExpr<int>("A")));
  EXPECT_FALSE(b.has_aggregate());

  b = a;
  EXPECT_TRUE(b.has_aggregate());
}

TEST(ValueTest, AssignmentOperatorWithoutAggregate) {
  Value<int> a(2);
  EXPECT_FALSE(a.has_aggregate());
  a.set_aggregate(2);
  EXPECT_TRUE(a.has_aggregate());

  Value<int> b(2, SharedExpr(new AnyExpr<int>("A")));
  EXPECT_FALSE(b.has_aggregate());

  a = b;
  EXPECT_FALSE(a.has_aggregate());
}

TEST(ValueTest, IsConcrete) {
  EXPECT_TRUE(Value<int>(2).is_concrete());
  EXPECT_TRUE(Value<int>(2, SharedExpr(new AnyExpr<int>("A"))).is_concrete());
  EXPECT_FALSE(Value<int>("A").is_concrete());
  EXPECT_FALSE(Value<int>(Value<int>("A")).is_concrete());
}

TEST(ValueTest, AssignmentWithConcreteValue) {
  Value<int> a(2);
  Value<int> b("A");

  a = b;
  EXPECT_TRUE(a.is_concrete());
}

TEST(ValueTest, AssignmentNonConcreteValue) {
  Value<int> a(2);
  Value<int> b("A");

  b = a;
  EXPECT_FALSE(b.is_concrete());
}
