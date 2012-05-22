#include <sstream>
#include "gtest/gtest.h"
#include "sp.h"

TEST(LoopTest, BoundedUnwindingPolicy) {
  const Value<bool> anything = any_bool("ANY");
  BoundedUnwindingPolicy policy(2);
  EXPECT_TRUE(policy.unwind(anything));
  EXPECT_TRUE(policy.unwind(anything));

  EXPECT_FALSE(policy.unwind(anything));

  EXPECT_FALSE(policy.unwind(anything));
  EXPECT_FALSE(policy.unwind(anything));
  // ...
}

TEST(LoopTest, LoopWithBoundedUnwindingPolicy) {
  const Value<bool> anything = any_bool("ANY");
  std::unique_ptr<UnwindingPolicy> policy_ptr(new BoundedUnwindingPolicy(2));
  Loop loop(std::move(policy_ptr));

  EXPECT_FALSE(policy_ptr);

  EXPECT_TRUE(loop.unwind(anything));
  EXPECT_TRUE(loop.unwind(anything));

  EXPECT_FALSE(loop.unwind(anything));

  EXPECT_FALSE(loop.unwind(anything));
  EXPECT_FALSE(loop.unwind(anything));
  // ...
}

TEST(LoopTest, LoopWithK) {
  const Value<bool> anything = any_bool("ANY");
  Loop loop(2);

  EXPECT_TRUE(loop.unwind(anything));
  EXPECT_TRUE(loop.unwind(anything));

  EXPECT_FALSE(loop.unwind(anything));

  EXPECT_FALSE(loop.unwind(anything));
  EXPECT_FALSE(loop.unwind(anything));
  // ...
}

TEST(LoopTest, VersionAfterUnwind1xWithSingleVar) {
  bool ok;
  Int i = any_int("I");
  Loop loop(1);
  loop.track(i);

  ok = loop.unwind(i < 5);
  EXPECT_TRUE(ok);

  i = i + 1;

  Version version = i.get_version();

  ok = loop.unwind(any_bool("ANY"));
  EXPECT_FALSE(ok);

  // version increase due to join operation
  EXPECT_EQ(version + 1, i.get_version());
}

TEST(LoopTest, Unwind1xWithSingleVar) {
  bool ok;
  Int i = any_int("I");
  Loop loop(1);
  loop.track(i);

  ok = loop.unwind(i < 5);
  EXPECT_TRUE(ok);

  i = i + 1;

  ok = loop.unwind(any_bool("ANY"));
  EXPECT_FALSE(ok);

  std::stringstream out;
  i.get_value().get_expr()->write(out);
  EXPECT_EQ("(([I]<5)?([I]+1):[I])", out.str());
}

TEST(LoopTest, Unwind2xWithSingleVar) {
  bool ok;
  Int i = any_int("I");
  Loop loop(2);
  loop.track(i);

  // 1x
  ok = loop.unwind(i < 5);
  EXPECT_TRUE(ok);

  i = i + 1;

  std::stringstream out_1x;
  i.get_value().get_expr()->write(out_1x);
  EXPECT_EQ("([I]+1)", out_1x.str());

  // 2x
  ok = loop.unwind(i < 7);
  EXPECT_TRUE(ok);

  i = i + 2;

  std::stringstream out_2x;
  i.get_value().get_expr()->write(out_2x);
  EXPECT_EQ("(([I]+1)+2)", out_2x.str());

  ok = loop.unwind(any_bool("ANY"));
  EXPECT_FALSE(ok);

  std::stringstream join_out;
  i.get_value().get_expr()->write(join_out);
  EXPECT_EQ("(([I]<5)?((([I]+1)<7)?(([I]+1)+2):([I]+1)):[I])", join_out.str());
}

TEST(LoopTest, Unwind2xWithMultipleVars) {
  bool ok;
  Int i = any_int("I");
  Int j = any_int("J");
  Loop loop(2);
  loop.track(i);
  loop.track(j);

  // 1x
  ok = loop.unwind(i < 5);
  EXPECT_TRUE(ok);

  i = i + 1;
  j = j + 1;

  std::stringstream out_i_1x;
  i.get_value().get_expr()->write(out_i_1x);
  EXPECT_EQ("([I]+1)", out_i_1x.str());

  std::stringstream out_j_1x;
  j.get_value().get_expr()->write(out_j_1x);
  EXPECT_EQ("([J]+1)", out_j_1x.str());

  // 2x
  ok = loop.unwind(i < 7);
  EXPECT_TRUE(ok);

  i = i + 2;
  j = j + 2;

  std::stringstream out_i_2x;
  i.get_value().get_expr()->write(out_i_2x);
  EXPECT_EQ("(([I]+1)+2)", out_i_2x.str());

  std::stringstream out_j_2x;
  j.get_value().get_expr()->write(out_j_2x);
  EXPECT_EQ("(([J]+1)+2)", out_j_2x.str());

  ok = loop.unwind(any_bool("ANY"));
  EXPECT_FALSE(ok);

  std::stringstream out_i_join;
  i.get_value().get_expr()->write(out_i_join);
  EXPECT_EQ("(([I]<5)?((([I]+1)<7)?(([I]+1)+2):([I]+1)):[I])", out_i_join.str());

  std::stringstream out_j_join;
  j.get_value().get_expr()->write(out_j_join);
  EXPECT_EQ("(([I]<5)?((([I]+1)<7)?(([J]+1)+2):([J]+1)):[J])", out_j_join.str());
}

TEST(LoopTest, MultipleTrack) {
  bool ok;
  Int i = any_int("I");
  Loop loop(2);

  loop.track(i);
  EXPECT_NO_THROW(loop.track(i));
}

