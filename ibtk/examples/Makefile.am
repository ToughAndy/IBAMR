## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/Make-rules
SUBDIRS = CCLaplace CCPoisson PhysBdryOps SCLaplace SCPoisson VCLaplace

## Standard make targets.
examples:
	@(cd CCLaplace   && $(MAKE) $(AM_MAKEFLAGS) $@) || exit 1;
	@(cd CCPoisson   && $(MAKE) $(AM_MAKEFLAGS) $@) || exit 1;
	@(cd PhysBdryOps && $(MAKE) $(AM_MAKEFLAGS) $@) || exit 1;
	@(cd SCLaplace   && $(MAKE) $(AM_MAKEFLAGS) $@) || exit 1;
	@(cd SCPoisson   && $(MAKE) $(AM_MAKEFLAGS) $@) || exit 1;
	@(cd VCLaplace   && $(MAKE) $(AM_MAKEFLAGS) $@) || exit 1;