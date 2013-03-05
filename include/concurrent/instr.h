// Copyright 2013, Alex Horn. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef LIBSE_CONCURRENT_INSTR_H_
#define LIBSE_CONCURRENT_INSTR_H_

#include <memory>
#include <cassert>
#include <utility>

#include "core/type.h"

#include "concurrent/event.h"

namespace se {

/// Non-copyable class that identifies a built-in memory read instruction

/// Any operands of a subclass of ReadInstr<T> must be only accessible
/// through a constant reference. If a subclass of ReadInstr<T> refers
/// to an event, then it must be of type ReadEvent<T>.
template<typename T>
class ReadInstr {
protected:
  ReadInstr() {}

public:
  ReadInstr(const ReadInstr& other) = delete;
  virtual ~ReadInstr() {}

  virtual std::shared_ptr<ReadInstr<bool>> condition_ptr() const = 0;
};

template<typename T>
class LiteralReadInstr : public ReadInstr<T> {
private:
  const T m_literal;
  const std::shared_ptr<ReadInstr<bool>> m_condition;

protected:
  std::shared_ptr<ReadInstr<bool>> condition_ptr() const {
    return m_condition;
  }

public:
  LiteralReadInstr(const T& literal,
    const std::shared_ptr<ReadInstr<bool>>& condition = nullptr) :
    m_literal(literal), m_condition(condition) {}

  LiteralReadInstr(const LiteralReadInstr& other) = delete;

  ~LiteralReadInstr() {}

  const T& literal() const { return m_literal; }
};

template<typename T>
class BasicReadInstr : public ReadInstr<T> {
private:
  std::shared_ptr<ReadEvent<T>> m_event_ptr;

protected:
  std::shared_ptr<ReadInstr<bool>> condition_ptr() const {
    return m_event_ptr->condition_ptr();
  }

public:
  BasicReadInstr(std::unique_ptr<ReadEvent<T>> event_ptr) :
    m_event_ptr(std::move(event_ptr)) {}

  BasicReadInstr(const BasicReadInstr& other) = delete;

  ~BasicReadInstr() {}

  std::shared_ptr<ReadEvent<T>> event_ptr() const { return m_event_ptr; }
};

template<Operator op, typename U>
class UnaryReadInstr : public ReadInstr<typename ReturnType<op, U>::result_type> {
private:
  const std::unique_ptr<ReadInstr<U>> m_operand_ptr;

protected:
  std::shared_ptr<ReadInstr<bool>> condition_ptr() const {
    return m_operand_ptr->condition_ptr();
  }

public:
  UnaryReadInstr(std::unique_ptr<ReadInstr<U>> operand_ptr) :
    m_operand_ptr(std::move(operand_ptr)) {}

  UnaryReadInstr(const UnaryReadInstr& other) = delete;

  ~UnaryReadInstr() {}

  const ReadInstr<U>& operand_ref() const {
    return *m_operand_ptr;
  }
};

template<Operator op, typename U, typename V>
class BinaryReadInstr :
  public ReadInstr<typename ReturnType<op, U, V>::result_type> {

private:
  const std::unique_ptr<ReadInstr<U>> m_loperand_ptr;
  const std::unique_ptr<ReadInstr<V>> m_roperand_ptr;

protected:
  std::shared_ptr<ReadInstr<bool>> condition_ptr() const {
    return m_loperand_ptr->condition_ptr();
  }

public:
  BinaryReadInstr(std::unique_ptr<ReadInstr<U>> loperand_ptr,
    std::unique_ptr<ReadInstr<V>> roperand_ptr) :
    m_loperand_ptr(std::move(loperand_ptr)),
    m_roperand_ptr(std::move(roperand_ptr)) {

    assert(m_loperand_ptr->condition_ptr() == m_roperand_ptr->condition_ptr());
  }

  BinaryReadInstr(const BinaryReadInstr& other) = delete;

  ~BinaryReadInstr() {}

  const ReadInstr<U>& loperand_ref() const { return *m_loperand_ptr; }
  const ReadInstr<V>& roperand_ref() const { return *m_roperand_ptr; }
};

}

#endif