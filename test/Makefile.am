testdir = $(srcdir)/test/src

TESTS = testHpLRU

check_PROGRAMS += $(TESTS)

tests: $(BUILT_SOURCES) $(TESTS) testsetup

test_LIBS = -lboost_system -lboost_thread libhplru.la

testHpLRU_SOURCES = $(testdir)/testHpLRU.cpp
testHpLRU_LDADD = $(test_LIBS)

.PHONY: testsetup
testsetup:
	mkdir -p testlogdir
