# Since everything we compile requires the same headers,
# we define the global AM_CPPFLAGS. We also require the
# C++11 standard to be able to use std::shared_ptr.
AM_CPPFLAGS = -std=c++0x -I$(srcdir)/include

AUTOMAKE_OPTIONS = foreign

# Always include gtest in distributions.
DIST_SUBDIRS = gtest

if HAVE_DOXYGEN

doc:
	$(DOXYGEN) $(top_srcdir)/doc/Doxyfile

endif

# The following local target definition is copied from the Protobuf project:
#   Build gtest before we build our tests. We don't add gtest to SUBDIRS
#   because then "make check" would also build and run all of gtest's own tests,
#   which takes a lot of time and is generally not useful to us. Also, we don't
#   want "make install" to recurse into gtest since we don't want to overwrite
#   the installed version of gtest if there is one.
check-local:
	@echo "Making lib/libgtest.a lib/libgtest_main.a in gtest"
	@cd gtest && $(MAKE) $(AM_MAKEFLAGS) lib/libgtest.la lib/libgtest_main.la

# We need to create a new custom target to run the tests because Autoconf does
# not reliably add check-local before the check target.
test: check-local check

bench: all
	git rev-parse HEAD >> bench/LOG
	time -p -o bench/LOG -a bench/for_loop
	time -p -o bench/LOG -a bench/while_loop

.PHONY: bench doc

# The following local target definition is copied from the Protobuf project:
#   We would like to clean gtest when "make clean" is invoked. But we have to
#   be careful because clean-local is also invoked during "make distclean", but
#   "make distclean" already recurses into gtest because it's listed among the
#   DIST_SUBDIRS. distclean will delete gtest/Makefile, so if we then try to
#   cd to the directory again and "make clean" it will fail. So, check that the
#   Makefile exists before recursing.
clean-local:
	@if test -e gtest/Makefile; then \
	  echo "Making clean in gtest"; \
	  cd gtest && $(MAKE) $(AM_MAKEFLAGS) clean; \
	fi

dist_doc_DATA = README.md

# Build rules for libraries.
lib_LTLIBRARIES = lib/libse.la

lib_libse_la_SOURCES = \
  src/op.cpp \
  src/expr.cpp \
  src/value.cpp \
  src/var.cpp \
  src/if.cpp \
  src/loop.cpp \
  src/tester.cpp \
  src/tracer.cpp

pkginclude_HEADERS = \
  include/op.h \
  include/value.h \
  include/expr.h \
  include/eval.h \
  include/instr.h \
  include/visitor.h \
  include/interpreter.h \
  include/var.h \
  include/if.h \
  include/loop.h \
  include/sequential-se.h \
  include/tester.h \
  include/tracer.h

# Build rules for functional and unit tests.
# Recall the Automake naming conventions:
#
# TESTS -- Programs run automatically by "make check"
# check_PROGRAMS -- Programs built by "make check" but not necessarily run

# See README in Z3 source code for installation instructions
tests_library_path = $(srcdir)/solvers/z3/build

TESTS_ENVIRONMENT = \
  LIBPATH=$(tests_library_path):$$LIBPATH \
  LD_LIBRARY_PATH=$(tests_library_path):$$LD_LIBRARY_PATH \
  DYLD_LIBRARY_PATH=$(tests_library_path):$$DYLD_LIBRARY_PATH \
  LIBRARY_PATH=$(tests_library_path):$$LIBRARY_PATH \
  SHLIB_PATH=$(tests_library_path):$$SHLIB_PATH \
  LD_LIBRARY64_PATH=$(tests_library_path):$$LD_LIBRARY64_PATH \
  LD_LIBRARYN32_PATH=$(tests_library_path):$$LD_LIBRARYN32_PATH \
  LD_LIBRARY_PATH_32=$(tests_library_path):$$LD_LIBRARY_PATH_32 \
  LD_LIBRARY_PATH_64=$(tests_library_path):$$LD_LIBRARY_PATH_64

TESTS = test/se
check_PROGRAMS = test/se

test_se_SOURCES = \
  test/op_test.cpp \
  test/value_test.cpp \
  test/any_test.cpp \
  test/expr_test.cpp \
  test/eval_test.cpp \
  test/instr_test.cpp \
  test/visitor_test.cpp \
  test/interpreter_test.cpp \
  test/var_test.cpp \
  test/if_test.cpp \
  test/loop_test.cpp \
  test/version_test.cpp \
  test/tracer_test.cpp \
  test/simplify_test.cpp \
  test/tester_functional_test.cpp \
  test/single_path_functional_test.cpp \
  test/multi_path_functional_test.cpp

test_se_CPPFLAGS = -std=c++0x -I$(srcdir)/include -I$(top_srcdir)/gtest/include
test_se_LDADD = $(top_builddir)/gtest/lib/libgtest_main.la lib/libse.la

# Benchmarks

bin_PROGRAMS = bench/for_loop bench/while_loop
bench_for_loop_SOURCES = bench/for_loop_bench.cpp
bench_for_loop_CPPFLAGS = -std=c++0x -I$(srcdir)/include
bench_for_loop_LDADD = lib/libse.la

bench_while_loop_SOURCES = bench/while_loop_bench.cpp
bench_while_loop_CPPFLAGS = -std=c++0x -I$(srcdir)/include
bench_while_loop_LDADD = lib/libse.la