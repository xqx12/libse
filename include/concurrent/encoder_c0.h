// Copyright 2013, Alex Horn. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef LIBSE_CONCURRENT_ENCODER_C0_H_
#define LIBSE_CONCURRENT_ENCODER_C0_H_

#include <string>

#include <z3++.h>

#include "concurrent/encoder.h"
#include "concurrent/instr.h"
#include "concurrent/relation.h"

namespace se {

class Z3C0 : public Z3 {
private:
  const z3::func_decl m_rf_func_decl;

  friend class Z3ValueEncoderC0;
  friend class Z3ReadEncoderC0;

  template<typename T> friend z3::expr ReadEvent<T>::constant(Z3C0&) const;
  template<typename T> friend z3::expr DirectWriteEvent<T>::constant(Z3C0&) const;

  template<typename T, typename U, size_t N>
  friend z3::expr IndirectWriteEvent<T, U, N>::constant(Z3C0&) const;

  unsigned join_id;

  z3::symbol create_symbol(const Event& event) {
    return context.int_symbol(event.event_id());
  }

  template<typename T, size_t N>
  z3::expr create_array_constant(const Event& event) {
    z3::sort domain_sort(context.bv_sort(TypeInfo<size_t>::s_type.bv_size()));
    z3::sort range_sort(context.bv_sort(TypeInfo<T>::s_type.bv_size()));
    z3::sort array_sort(context.array_sort(domain_sort, range_sort));

    return context.constant(create_symbol(event), array_sort);
  }

  z3::expr constant(const ReadEvent<bool>& event) {
    return context.constant(create_symbol(event), context.bool_sort());
  }

  template<typename T, size_t N>
  z3::expr constant(const ReadEvent<T[N]>& event) {
    return create_array_constant<T, N>(event);
  }

  template<typename T, size_t N>
  z3::expr constant(const WriteEvent<T[N]>& event) {
    return create_array_constant<T, N>(event);
  }

  template<typename T, typename U, size_t N>
  z3::expr constant(const IndirectWriteEvent<T, U, N>& event) {
    return create_array_constant<T, N>(event);
  }

public:
  using Z3::context;
  using Z3::solver;

  Z3C0() : Z3(), m_rf_func_decl(context.function("rf",
    /* domain */ context.int_sort(), /* range */ context.int_sort())),
    join_id(0) {}

  z3::expr constant(const Event& event) {
    z3::sort sort(context.bv_sort(event.type().bv_size()));
    return context.constant(create_symbol(event), sort);
  }

  bool is_clock(const z3::expr& expr) {
    return expr.is_int();
  }

  z3::sort clock_sort() {
    return context.int_sort();
  }

  z3::expr happens_before(const z3::expr& x, const z3::expr& y) {
    assert(is_clock(x) && is_clock(y));
    return x < y;
  }

  z3::expr join_clocks(const z3::expr& x, const z3::expr& y) {
    assert(is_clock(x) && is_clock(y));

    const std::string join_name = std::to_string(join_id++) + "_Join";
    z3::symbol join_clock_symbol(context.str_symbol(join_name.c_str()));
    z3::expr join_clock(context.constant(join_clock_symbol, clock_sort()));
    solver.add(join_clock > 0);
    solver.add(happens_before(x, join_clock) && happens_before(y, join_clock));
    return join_clock;
  }

  z3::expr rf(const Event& write_event, const Event& read_event) {
    assert(write_event.is_write());
    assert(read_event.is_read());

    const z3::expr read_event_id(context.int_val(read_event.event_id()));
    const z3::expr write_event_id(context.int_val(write_event.event_id()));
    return write_event_id == m_rf_func_decl(read_event_id);
  }

  z3::expr clock(const Event& event) {
    z3::expr clock(context.constant(create_symbol(event), clock_sort()));
    solver.add(clock > 0);
    return clock;
  }

  /// Individual array element literal
  template<typename T, size_t N = std::extent<T>::value,
    class = typename std::enable_if<std::is_array<T>::value and 0 < N>::type>
  z3::expr literal(const LiteralReadInstr<T>& instr) {

    typedef typename std::remove_extent<T>::type ElementType;
    return literal(LiteralReadInstr<ElementType>(instr.element_literal()));
  }

  z3::expr literal(const LiteralReadInstr<bool>& instr) {
    return context.bool_val(instr.literal());
  }

  template<typename T, class = typename std::enable_if<
    std::is_arithmetic<T>::value>::type>
  z3::expr literal(const LiteralReadInstr<T>& instr) {

    const char* str = std::to_string(instr.literal()).c_str();
    return context.bv_val(str, TypeInfo<T>::s_type.bv_size());
  }
};

/// EncoderC0 for read instructions 
class Z3ReadEncoderC0 {
private:

public:
  Z3ReadEncoderC0() {}

  template<typename T>
  z3::expr encode(const LiteralReadInstr<T>& instr, Z3C0& helper) const {
    return helper.literal(instr);
  }

  template<typename T>
  z3::expr encode(const BasicReadInstr<T>& instr, Z3C0& helper) const {
    return helper.constant(*instr.event_ptr());
  }

  template<Opcode opcode, typename T>
  z3::expr encode(const UnaryReadInstr<opcode, T>& instr, Z3C0& helper) const {
    return Eval<opcode>::eval(instr.operand_ref().encode(*this, helper));
  }

  template<Opcode opcode, typename T, typename U>
  z3::expr encode(const BinaryReadInstr<opcode, T, U>& instr, Z3C0& helper) const {
    return Eval<opcode>::eval(instr.loperand_ref().encode(*this, helper),
      instr.roperand_ref().encode(*this, helper));
  }

  template<Opcode opcode, typename T>
  z3::expr encode(const NaryReadInstr<opcode, T>& instr, Z3C0& helper) const {
    z3::expr nary_expr = Z3Identity<opcode, T>::constant(helper.context);
    for (const std::shared_ptr<ReadInstr<T>>& operand_ptr : instr.operand_ptrs()) {
      nary_expr = Eval<opcode>::eval(nary_expr, operand_ptr->encode(*this, helper));
    }
    return nary_expr;
  }

  template<typename T, typename U, size_t N>
  z3::expr encode(const DerefReadInstr<T[N], U>& instr, Z3C0& helper) const {
    return z3::select(instr.memory_ref().encode(*this, helper),
      instr.offset_ref().encode(*this, helper));
  }
};

#define READ_ENCODER_C0_FN \
  encode(const Z3ReadEncoderC0& encoder, Z3C0& helper) const {\
    return encoder.encode(*this, helper);\
  }

template<typename T> z3::expr LiteralReadInstr<T>::READ_ENCODER_C0_FN
template<typename T, size_t N> z3::expr LiteralReadInstr<T[N]>::READ_ENCODER_C0_FN
template<typename T> z3::expr BasicReadInstr<T>::READ_ENCODER_C0_FN

template<Opcode opcode, typename T>
z3::expr UnaryReadInstr<opcode, T>::READ_ENCODER_C0_FN

template<Opcode opcode, typename T, typename U>
z3::expr BinaryReadInstr<opcode, T, U>::READ_ENCODER_C0_FN

template<Opcode opcode, typename T>
z3::expr NaryReadInstr<opcode, T>::READ_ENCODER_C0_FN

template<typename T, typename U, size_t N>
z3::expr DerefReadInstr<T[N], U>::READ_ENCODER_C0_FN

/// EncoderC0 for a read instruction in the left-hand side of an assignment
class Z3WriteEncoderC0 {
public:
  Z3WriteEncoderC0() {}
  
  template<typename T, typename U, size_t N>
  z3::expr encode(const DerefReadInstr<T[N], U>& instr,
    const Z3ReadEncoderC0& read_encoder,
    const z3::expr& rhs_expr, Z3C0& helper) const {

    return z3::store(instr.memory_ref().encode(read_encoder, helper),
      instr.offset_ref().encode(read_encoder, helper), rhs_expr);
  }
};

/// EncoderC0 for the values of direct and indirect write events

/// Every `encode_eq(...)` member function returns a Z3 expression whose sort
/// is Boolean.
class Z3ValueEncoderC0 {
private:
  const Z3WriteEncoderC0 m_write_encoder;
  const Z3ReadEncoderC0 m_read_encoder;

public:
  Z3ValueEncoderC0() : m_read_encoder(), m_write_encoder() {}

  template<typename T>
  z3::expr encode_eq(const ReadEvent<T>& event, Z3C0& helper) const {
    return helper.context.bool_val(false);
  }

  z3::expr encode_eq(const SyncEvent& event, Z3C0& helper) const {
    return helper.context.bool_val(true);
  }

  template<typename T>
  z3::expr encode_eq(const DirectWriteEvent<T>& event, Z3C0& helper) const {
    z3::expr lhs_expr(helper.constant(event));
    z3::expr rhs_expr(event.instr_ref().encode(m_read_encoder, helper));
    return lhs_expr == rhs_expr;
  }

  template<typename T, size_t N>
  z3::expr encode_eq(const DirectWriteEvent<T[N]>& event, Z3C0& helper) const {
    z3::expr lhs_expr(helper.constant(event));
    z3::sort domain_sort(lhs_expr.get_sort().array_domain());
    z3::expr init_expr(event.instr_ref().encode(m_read_encoder, helper));
    z3::expr rhs_expr(z3::const_array(domain_sort, init_expr));
    return lhs_expr == rhs_expr;
  }

  template<typename T, typename U, size_t N>
  z3::expr encode_eq(const IndirectWriteEvent<T, U, N>& event, Z3C0& helper) const {
    z3::expr lhs_expr(helper.constant(event));
    z3::expr rhs_expr(event.instr_ref().encode(m_read_encoder, helper));
    return lhs_expr == m_write_encoder.encode(event.deref_instr_ref(),
      m_read_encoder, rhs_expr, helper);
  }

  template<typename T>
  z3::expr encode_eq(std::unique_ptr<ReadInstr<T>> instr_ptr, Z3C0& z3) const {
    return instr_ptr->encode(m_read_encoder, z3);
  }
};

#define VALUE_ENCODER_C0_FN \
  encode_eq(const Z3ValueEncoderC0& encoder, Z3C0& helper) const {\
    return encoder.encode_eq(*this, helper);\
  }

#define CONSTANT_ENCODER_C0_FN \
  constant(Z3C0& helper) const { return helper.constant(*this); }

template<typename T>
z3::expr ReadEvent<T>::VALUE_ENCODER_C0_FN

template<typename T>
z3::expr ReadEvent<T>::CONSTANT_ENCODER_C0_FN

template<typename T>
z3::expr DirectWriteEvent<T>::VALUE_ENCODER_C0_FN

template<typename T>
z3::expr DirectWriteEvent<T>::CONSTANT_ENCODER_C0_FN

template<typename T, typename U, size_t N>
z3::expr IndirectWriteEvent<T, U, N>::VALUE_ENCODER_C0_FN

template<typename T, typename U, size_t N>
z3::expr IndirectWriteEvent<T, U, N>::CONSTANT_ENCODER_C0_FN

class Z3OrderEncoderC0 {
private:
  const Z3ReadEncoderC0 m_read_encoder;

  z3::expr event_condition(const Event& event, Z3C0& z3) const {
    if (event.condition_ptr()) {
      return event.condition_ptr()->encode(m_read_encoder, z3);
    }

    return z3.context.bool_val(true);
  }

  typedef std::shared_ptr<Event> EventPtr;
  typedef std::unordered_set<EventPtr> EventPtrSet;

public:
  Z3OrderEncoderC0() : m_read_encoder() {}

  /// \internal \return RF axiom encoding
  z3::expr rf_enc(const ZoneRelation<Event>& relation, Z3C0& z3) const {
    z3::expr rf_expr(z3.context.bool_val(true));
    for (const EventPtr& x_ptr : /* read events */ relation.event_ptrs()) {
      if (x_ptr->is_write()) { continue; }
      const Event& read_event = *x_ptr;

      assert(!read_event.zone().is_bottom());
      const z3::expr read_event_condition(event_condition(read_event, z3));

      z3::expr wr_schedules(z3.context.bool_val(false));
      for (const EventPtr& y_ptr : /* write events */ relation.event_ptrs()) {
        if (y_ptr->is_read()) { continue; }
        const Event& write_event = *y_ptr;

        assert(!write_event.zone().is_bottom());
        if (read_event.zone().meet(write_event.zone()).is_bottom()) { continue; }

        const z3::expr wr_order(
          z3.happens_before(z3.clock(write_event), z3.clock(read_event)));

        const z3::expr wr_schedule(z3.rf(write_event, read_event));
        const z3::expr wr_equality(write_event.constant(z3) ==
          read_event.constant(z3));
        const z3::expr write_event_condition(event_condition(write_event, z3));

        wr_schedules = wr_schedules or wr_schedule;
        rf_expr = rf_expr and implies(wr_schedule, wr_order and
          write_event_condition and read_event_condition and wr_equality);
      }

      rf_expr = rf_expr and implies(read_event_condition, wr_schedules);
    }
    return rf_expr;
  }

  /// \internal \return WS axiom encoding
  z3::expr ws_enc(const ZoneRelation<Event>& relation, Z3C0& z3) const {
    const ZoneAtomSet& zone_atoms = relation.zone_atoms();

    z3::expr ws_expr(z3.context.bool_val(true));
    for (const Zone& zone : zone_atoms) {
      const EventPtrSet write_event_ptrs = relation.find(zone,
        WriteEventPredicate::predicate());

      const unsigned zone_write_event_count = write_event_ptrs.size();
      Z3_ast args[zone_write_event_count];
      unsigned i = 0;
      for (const EventPtr& write_event_ptr : write_event_ptrs) {
        const Event& write_event = *write_event_ptr;
        args[i] = z3.clock(write_event);
        Z3_inc_ref(z3.context, args[i]);

        i++;
      }

      const z3::expr zone_ws_expr(z3.context,
        Z3_mk_distinct(z3.context, zone_write_event_count, args));

      for (i = 0; i < zone_write_event_count; i++) {
        Z3_dec_ref(z3.context, args[i]);
      }

      ws_expr = ws_expr and zone_ws_expr;
    }

    return ws_expr;
  }

  /// \internal \return FR axiom encoding
  z3::expr fr_enc(const ZoneRelation<Event>& relation, Z3C0& z3) const {
    const ZoneAtomSet& zone_atoms = relation.zone_atoms();

    z3::expr fr_expr(z3.context.bool_val(true));
    for (const Zone& zone : zone_atoms) {
      const std::pair<EventPtrSet, EventPtrSet> result =
        relation.partition(zone);
      const EventPtrSet& read_event_ptrs = result.first;
      const EventPtrSet& write_event_ptrs = result.second;

      for (const EventPtr& write_event_ptr_x : write_event_ptrs) {
        for (const EventPtr& write_event_ptr_y : write_event_ptrs) {
          if (write_event_ptr_x == write_event_ptr_y) { continue; }

          const Event& write_event_x = *write_event_ptr_x;
          const Event& write_event_y = *write_event_ptr_y;

          assert(!write_event_x.zone().is_bottom());
          assert(!write_event_y.zone().is_bottom());

          for (const EventPtr& read_event_ptr : read_event_ptrs) {
            const Event& read_event = *read_event_ptr;

            assert(!read_event.zone().is_bottom());

            const z3::expr xr_schedule(z3.rf(write_event_x, read_event));
            const z3::expr xy_order(z3.happens_before(z3.clock(write_event_x), z3.clock(write_event_y)));
            const z3::expr ry_order(z3.happens_before(z3.clock(read_event), z3.clock(write_event_y)));
            const z3::expr y_condition(event_condition(write_event_y, z3));

            fr_expr = fr_expr and
              implies(xr_schedule and xy_order and y_condition, ry_order);
          }
        }
      }
    }

    return fr_expr;
  }
};

}

#endif
