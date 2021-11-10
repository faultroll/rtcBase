
prfx   ?= # arm-himix200-linux-
cxx    := $(prfx)g++
ar     := $(prfx)ar
ranlib := $(prfx)ranlib
strip  := $(prfx)strip

# c version will be rtcthrd_c
name    := rtcthrd
srcs    := $(wildcard rtc_base/*.cc) \
           $(wildcard rtc_base/synchronization/*.cc)
objs    := $(patsubst %.cc,%.o,$(filter %.cc, $(srcs)))
deps    := $(patsubst %.o,%.d,$(objs))
libs    := -lpthread
cflags   = -I. -DWEBRTC_POSIX # -DWEBRTC_WIN
cflags  += -g # -DNDEBUG
ldflags := 
# for reproducible build
objs    := $(sort $(objs))
# cflags  += -Wno-builtin-macro-redefined -U__FILE__ -D__FILE__=\"$(notdir $<)\"

targets := lib$(name).so lib$(name).a
demos   := single.elf multi.elf override.elf projects.elf
all : $(targets) $(demos)

clean : 
	rm -f $(targets) $(demos)
	rm -f $(objs) $(deps)

lib$(name).so : $(objs)
	@$(cxx) -shared -Wl,--gc-sections -Wl,--as-needed -Wl,--export-dynamic $(ldflags) $^ -o $@ $(libs)
	@$(strip) --strip-all $@
	$(info $(cxx) -shared $(notdir $^) -o $(notdir $@))

lib$(name).a : $(objs)
	@$(ar) -crD $@ $^
	@$(ranlib) -D $@
	@$(strip) --strip-unneeded $@
	$(info $(ar) -crD $(notdir $@) $(notdir $^))

%.o : %.cc
	@$(cxx) -Os -Wall -Wextra -fPIC -std=c++11 -fpermissive $(cflags) -c $< -o $@ -MMD -MF $*.d -MP
	$(info $(cxx) -c $(notdir $<) -o $(notdir $@))

-include $(deps)

%.elf : demo/%.o lib$(name).a
	@$(cxx) $(cflags) -Wl,--gc-sections -Wl,--as-needed -Wl,--export-dynamic $(ldflags) $^ -o $@ $(libs)
