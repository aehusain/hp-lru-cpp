
## Additional flags to pass to aclocal when it is invoked automatically at
## make time. The ${ACLOCAL_FLAGS} variable is picked up from the environment
## to provide a way for the user to supply additional arguments.
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

AM_CFLAGS =
AM_CPPFLAGS = -std=c++0x
AM_CXXFLAGS = -Wall -Werror


INCLUDE_DIR = /usr/local/include
BOOST_INCLUDE_DIR = /usr/local/include/boost
HPLRU_INCLUDE = $(srcdir)/include
BOOST_LIBS = -lboost_date_time -lboost_filesystem -lboost_graph  -lboost_math_c99 -lboost_math_c99f -lboost_math_c99l \
-lboost_math_tr1 -lboost_math_tr1f -lboost_math_tr1l -lboost_prg_exec_monitor -lboost_program_options -lboost_regex \
-lboost_serialization -lboost_signals -lboost_system -lboost_test_exec_monitor -lboost_thread  -lboost_unit_test_framework \
-lboost_wave -lboost_wserialization


LIBS = -lglog  -lpthread
if OS_IS_CENTOS
LIBS += -lbz2
endif

check_PROGRAMS =
bin_PROGRAMS =

INCS = -I$(BOOST_INCLUDE_DIR) -I$(INCLUDE_DIR) -I. \
       -I$(HPLRU_INCLUDE)

lib_LTLIBRARIES = libhplru.la

AM_CPPFLAGS += -DHAVE_NETINET_IN_H $(INCS)

AM_LDFLAGS =

libhplru_la_CPPFLAGS = -shared $(AM_CPPFLAGS)
libhplru_la_CXXFLAGS =  $(AM_CXXFLAGS)
libhplru_la_LDFLAGS = -module -no-undefined -avoid-version -shared $(BOOST_LIBS) $(LIBS) $(AM_LDFLAGS)
libhplru_la_SOURCES = src/HpLru.cpp util/ThreadPool.cpp util/Latch.cpp

include $(srcdir)/test/Makefile.am

## Define an independent executable script for inclusion in the distribution
## archive. It will not be installed on an end user's system, however.
dist_noinst_SCRIPTS = autogen.sh

COVERAGE_OUT_DIR = coverage_output
LCOV = lcov
GCOV = gcov
GENHTML = genhtml

.PHONY: libs
libs: $(BUILT_SOURCES) library_config.h hplru_config.h $(LTLIBRARIES)

all-local:

.PHONY: coverage
coverage:
	rm -rf $(COVERAGE_OUT_DIR)
	@mkdir -p $(COVERAGE_OUT_DIR)
	$(LCOV) --quiet --capture \
                --gcov-tool $(GCOV) \
                --directory . \
                --base-directory .\
                --rc lcov_branch_coverage=1\
                --output-file $(COVERAGE_OUT_DIR)/coverage.output
	$(GENHTML) -o $(COVERAGE_OUT_DIR) $(COVERAGE_OUT_DIR)/coverage.output


clean-local: clean-coverage

.PHONY: clean-coverage
clean-coverage:
	find . \( -name '*.covo' \
                  -o -name '*.gcno' \
                  -o -name '*.gcda' \) -print0 |xargs -0 rm -f
	rm -rf $(COVERAGE_OUT_DIR)
