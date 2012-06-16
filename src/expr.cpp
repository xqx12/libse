// Copyright 2012, Alex Horn. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "expr.h"

namespace se {

const Operator UNARY_BEGIN = NOT;
const Operator UNARY_END = ADD;
const Operator NARY_BEGIN = ADD;
const Operator NARY_END = LSS;

std::ostream& CastExpr::write(std::ostream& out) const {
  out << LPAR << LPAR;
  out << types[type];
  out << RPAR << LPAR;
  expr->write(out);
  out << RPAR << RPAR;
  return out;
}

std::ostream& UnaryExpr::write(std::ostream& out) const {
  out << LPAR;
  out << operators[op];
  expr->write(out);
  out << RPAR;
  return out;
}

std::ostream& TernaryExpr::write(std::ostream& out) const {
  out << LPAR;
  cond_expr->write(out);
  out << QUERY;
  then_expr->write(out);
  out << COLON;
  else_expr->write(out);
  out << RPAR;
  return out;
}

std::ostream& NaryExpr::write(std::ostream& out) const {
  out << LPAR;
  std::list<SharedExpr>::const_iterator iter = exprs.cbegin();
  (*iter)->write(out);
  for(iter++; iter != exprs.cend(); iter++) {
    out << operators[op];
    (*iter)->write(out);
  }
  out << RPAR;
  return out;
}

}

