# This file is heavily inspired by the example provided here
# https://gist.github.com/dvdfreitag/70ca95aae0b50cdff7f4cc87649a52ce
TARGET = sort-test

CXX = clang++
# c++20 for various type deductions
CFLAGS = --std=c++20 -Wall -Wextra -pedantic -O3

# the root (make) dir is the source dir here
SRCS := $(wildcard *.cpp)

# all object files will be placed to OBJDIR
OBJDIR = .build
OBJS := $(SRCS:%.cpp=$(OBJDIR)/%.o)

# the dependencies will be automatically detected (DEPFLAGS)
# and placed into DEPDIR directory
DEPDIR = .dep
DEPS := $(SRCS:%.cpp=$(DEPDIR)/%.d)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

# this defines targets, which are not associated with files
.PHONY: all clean pristine

all: $(TARGET)

# $@ - target
# $< - the first prerequisite
$(OBJDIR)/%.o: %.cpp | $(OBJDIR) $(DEPDIR)
	@echo [Compiling] $@
	@$(CXX) -c $(CFLAGS) $(DEPFLAGS) -o $@ $<

# $^ - all prerequisites
$(TARGET): $(OBJS)
	@echo [Linking] $@
	@$(CXX) $(LDFLAGS) -o $@ $^

# clean directories and remove the target
clean:
	@rm $(OBJDIR)/* $(DEPDIR)/* $(TARGET)

# return to the original state (no generated files nor directories)
pristine:
	@rm -rf $(OBJDIR) $(DEPDIR) $(TARGET)

$(OBJDIR) $(DEPDIR):
	@mkdir -p $@

# include dependency-determined rules
# why it doesn't lead to double rules, I don't know
include $(wildcard $(DEPS))
