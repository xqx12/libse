#include <sstream>
#include "gtest/gtest.h"
#include "overload.h"

TEST(VersionTest, Init) {
  Bool a = true;
  Bool b = true;
  Char c = 'a';
  Int d = 3;

  EXPECT_EQ(0, a.get_version());
  EXPECT_EQ(0, b.get_version());
  EXPECT_EQ(0, c.get_version());
  EXPECT_EQ(0, d.get_version());
}

TEST(VersionTest, InitWithAny) {
  Bool a = any_bool();
  Char b = any_char();
  Int c = any_int();

  EXPECT_EQ(0, a.get_version());
  EXPECT_EQ(0, b.get_version());
  EXPECT_EQ(0, c.get_version());
}

TEST(VersionTest, GetVersion) {
  Int a = 3;
  Int b = any_int();
  Char c = 'c';

  EXPECT_EQ(0, a.get_version());
  EXPECT_EQ(0, b.get_version());
  EXPECT_EQ(0, c.get_version());

  a = b + 2 + c;

  EXPECT_EQ(1, a.get_version());
  EXPECT_EQ(0, b.get_version());
  EXPECT_EQ(0, c.get_version());

  a = 4 + b + c;

  EXPECT_EQ(2, a.get_version());
  EXPECT_EQ(0, b.get_version());
  EXPECT_EQ(0, c.get_version());

  c = a + b;

  EXPECT_EQ(2, a.get_version());
  EXPECT_EQ(0, b.get_version());
  EXPECT_EQ(1, c.get_version());

  b = 5;

  EXPECT_EQ(2, a.get_version());
  EXPECT_EQ(1, b.get_version());
  EXPECT_EQ(1, c.get_version());

  b = a;

  EXPECT_EQ(2, a.get_version());
  EXPECT_EQ(2, b.get_version());
  EXPECT_EQ(1, c.get_version());

  b = c;

  EXPECT_EQ(2, a.get_version());
  EXPECT_EQ(3, b.get_version());
  EXPECT_EQ(1, c.get_version());
}

TEST(VersionTest, SelfAssignmentAfterInit) {
  Int a = 3;
  Int b = any_int();
  Char c = 'c';

  EXPECT_EQ(0, a.get_version());
  EXPECT_EQ(0, b.get_version());
  EXPECT_EQ(0, c.get_version());

  a = a;
  b = b;
  c = c;

  EXPECT_EQ(0, a.get_version());
  EXPECT_EQ(0, b.get_version());
  EXPECT_EQ(0, c.get_version());
}


TEST(VersionTest, SelfAssignmentAfterOperations) {
  Int a = 3;
  Int b = any_int();
  Char c = 'c';

  EXPECT_EQ(0, a.get_version());
  EXPECT_EQ(0, b.get_version());
  EXPECT_EQ(0, c.get_version());

  // some operations
  a = a + b + c;
  b = a + b + c;
  b = 5;
  c = c + 'x';
  c = 'y';
  c = 'z' + c;

  EXPECT_EQ(1, a.get_version());
  EXPECT_EQ(2, b.get_version());
  EXPECT_EQ(3, c.get_version());

  a = a;
  b = b;
  c = c;

  EXPECT_EQ(1, a.get_version());
  EXPECT_EQ(2, b.get_version());
  EXPECT_EQ(3, c.get_version());
}

