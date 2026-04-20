###################################################################
###################################################################
##                                                               ##
## Copyright (C) Microsoft Corporation. All rights reserved.     ##
##                                                               ##
## MspkMakeUtils.mk                                              ##
##                                                               ##
## Provides some additional common build logic that must appear  ##
## after the default rules.                                      ##
##                                                               ##
###################################################################
###################################################################


###############################################################################################
#
# AUTOMATIC DEPENDENCY TRACKING
#
# This is off by default.  Enable with:
#
#   export MSPK_BUILDOPTIONS="$MSPK_BUILDOPTIONS AUTODEPEND:ON"
#
# Each compiled .o will have a corresponding .d that is a compiler-generated
# set of dependencies.  "-include" is used instead of "include" because the .d
# file might not have been built yet (imagine a clean tree).
#
# The variable "AUTODEPEND_CPPFLAGS" contains the extra magic to make the
# compiler generate these dependency fles, and the compilation rules in
# MspkBase.mk already pass the value of this to the compiler.  Note that the
# variable will default to empty if this feature is not enabled, or if the
# file if this utility file is never included.  The depenency generation logic
# is implemented inside of GCC, and is merely enabled by passing some
# additional arguments to GCC.  "-MD" tells the compiler to generate the
# dependency file, using a name the same as the object except replacing the
# ".o" with ".d".  The -MP option tells the compiler to generate additional
# empty dependencies for each header file, which prevents the scenario of
# somebody removing a header file and the build erroring out with "Don't know
# how to build foo.h".
#
# You should add "$(AUTODEPEND_CPPFLAGS)" wherever you compile source code to
# an object file with GCC/G++.  Note, though, that it should be passed on the
# compiler invocation that will run the preprocessor.  This is important
# because some of the platforms have a build option that will run the compiler
# in multiple passes in order to leave behind preprocessed files and assembly
# files: in those cases you want to specify "$(AUTODEPEND_CPPFLAGS)" on the
# initial pass that will run the preprocessor, but not the other passes that
# generate the .s or .o files.  Note that even those these arguments are
# specific to the preprocessor pass, they use syntax that is understood by the
# "gcc"/"g++" front ends, and is not necessarily understook if you were to
# directly invoke the preprocessor.
#
# This code will also make each .o file depend on the makefiles, so that a
# change to the makefiles will cause a rebuild.  See below for how to disable
# if you are trying to debug the primary autodepend logic.
#
# AFAIK There are two remaining things that the dependency tracking doesn't
# track.  These are considered less common cases and it is assumed that
# developers will remember to "make clean".
#
# * Imagine FOO.C includes BAR.H.  If that dependency is known, and somebody
#   puts a new BAR.H at a location earlier in the include path, then a rebuild
#   should happen, yet will not.  Similarly, if BAR.H is removed, and there is
#   a different BAR.H further down in the include path, a rebuild is necessary
#   but will not happen.
#
#   The basic idea to fix this is to get the list of #includes for FOO.C, and
#   then generate an NxM set of depenencies where N is the #included names, M
#   is the directories in the include path.  But then you need to mix in all
#   the sub-included files.  This actually still needs to come from the
#   compiler: just ask the preprocessor to generate dependencies based on what
#   it _tried_ to open, not on what it finally opened.
#
# * Changes in environment variables.
#
#   Imagine for example that somebody changes "CFLAGS" in their environment to
#   tweak the build.  That should cause a rebuild since the compiler flags
#   have changed, yet will not.
#
#   If there was a reliable way to track all the environment variables used,
#   those could be persisted to a file.  Actually, write them to a temp file,
#   then do "move-if-change", so that the persisted environment variables get
#   their file updated only if there is a change.  Then just make all the
#   generated bits depend on that file.  Perhaps only the generated objects,
#   as the other stuff (libs, exes) should then fall into place.

AUTODEPEND_CPPFLAGS=
ifdef MSPKOPTION_AUTODEPEND

-include $(patsubst %.o,%.d,$(OBJS))
#$(warning *************************$(patsubst %.o,%.d,$(OBJS)))

AUTODEPEND_CPPFLAGS=-MD -MP -MT $(@)

# Make the objects depend on the makefiles, so that changing makefile logic
# causes a rebuild.  Note that there is a way to disable this, primarily for
# use by makefile maintainters so that they can work without rebuilding
# everything just because of this.  The autogenerated dependency files should
# not be in this list, otherwise things will rebuild all the time.
ifndef MSPK_NO_MAKEFILE_DEPENDENCIES
$(OBJS) : $(filter-out $(patsubst %.o,%.d,$(OBJS)),$(MAKEFILE_LIST))
endif

endif # MSPKOPTION_AUTODEPEND

##############################################################################
#
# GENERATE PREPROCESSED FILES
#
# It is often desirable to run the source code through the preprocessor in
# order to help debug compiler errors.
#
# These rules add additional targets so that you can request this on a
# per-file basis.
#
# Local to the directory, you can do a "make foo.cpp.pp" (the source file name
# with ".pp" added), to generate the preprocessed output.  Note that the file
# will appear in the source directory, not the object file directory.
#
# Similarly, "make foo.cpp.ppd" will generate preprocessed output that also
# includes "#define" directives to assist in locating where problematic macros
# are being defined.


#
# Appending ".pp" to a source file name will produce preprocessed output.
#
.PHOHY: $(addsuffix .pp,$(SOURCES))  # not really phony, but ensures it always runs
$(addsuffix .pp,$(SOURCES)): %.pp: %
	@$(call mspk.progress,CPP,$@,$<,$?)
	$(CXX) $(CXXFLAGS) $^ -o $(@) -E


#
# Appending ".ppd" to a source file name will produce preprocessed output with
# the #defines still present
#
.PHOHY: $(addsuffix .ppd,$(SOURCES))  # not really phony, but ensures it always runs
$(addsuffix .ppd,$(SOURCES)): %.ppd: %
	@$(call mspk.progress,CPP,$@,$<,$?)
	$(CXX) $(CXXFLAGS) $^ -o $(@) -E -dD


