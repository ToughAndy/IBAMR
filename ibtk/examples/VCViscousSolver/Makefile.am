## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/Make-rules

## Dimension-dependent testers
EXAMPLE_DRIVER = ex_main.cpp
GTEST_DRIVER   = test_main.cpp
EXTRA_DIST     = README input2d.periodic input3d.periodic
EXTRA_DIST    += input2d.test input3d.test

EXTRA_PROGRAMS =
EXAMPLES =
GTESTS   =
if SAMRAI2D_ENABLED
EXAMPLES += main2d
if GTEST_ENABLED
GTESTS   += test2d
endif
EXTRA_PROGRAMS += $(EXAMPLES) $(GTESTS)
endif
if SAMRAI3D_ENABLED
EXAMPLES += main3d
if GTEST_ENABLED
GTESTS   += test3d
endif
EXTRA_PROGRAMS += $(EXAMPLES) $(GTESTS)
endif

main2d_CXXFLAGS = $(AM_CXXFLAGS) -DNDIM=2
main2d_LDADD = $(IBTK_LDFLAGS) $(IBTK2d_LIBS) $(IBTK_LIBS)
main2d_SOURCES = $(EXAMPLE_DRIVER)

main3d_CXXFLAGS = $(AM_CXXFLAGS) -DNDIM=3
main3d_LDADD = $(IBTK_LDFLAGS) $(IBTK3d_LIBS) $(IBTK_LIBS)
main3d_SOURCES = $(EXAMPLE_DRIVER)

test2d_CXXFLAGS = $(AM_CXXFLAGS) -DNDIM=2
test2d_LDADD = $(IBTK_LDFLAGS) $(IBTK2d_LIBS) $(IBTK_LIBS)
test2d_SOURCES = $(GTEST_DRIVER)

test3d_CXXFLAGS = $(AM_CXXFLAGS) -DNDIM=3
test3d_LDADD = $(IBTK_LDFLAGS) $(IBTK3d_LIBS) $(IBTK_LIBS)
test3d_SOURCES = $(GTEST_DRIVER)

examples: $(EXAMPLES)
	if test "$(top_srcdir)" != "$(top_builddir)" ; then \
	  cp -f $(srcdir)/{README,input[23]d.periodic} $(PWD) ; \
	fi ;

if GTEST_ENABLED
gtest: $(GTESTS)
	if test "$(top_srcdir)" != "$(top_builddir)" ; then \
	  cp -f $(srcdir)/{README,input[23]d.periodic*} $(PWD) ; \
	fi ;
	./test2d input2d.test

gtest-long:
	make gtest
	./test3d input3d.test

else
gtest:
	@echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@echo " To run tests, Google Test Framework must be enabled.                "
	@echo " Configure IBAMR with additional options:                            "
	@echo "                      --enable-gtest --with-gtest=path               "
	@echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
gtest-long:
	make gtest
endif

clean-local:
	rm -f $(EXTRA_PROGRAMS)
	if test "$(top_srcdir)" != "$(top_builddir)" ; then \
	  rm -f $(builddir)/{README,input[23]d.periodic} ; \
	fi ;
