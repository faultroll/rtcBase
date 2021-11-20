
# arm-linux-gnueabi- # aarch64-linux-gnu-
prfx   ?= 
cc     := $(prfx)gcc
cxx    := $(prfx)g++
ar     := $(prfx)ar
ranlib := $(prfx)ranlib
strip  := $(prfx)strip

name    := rtc_audio
include rtc_base/SOURCE.mk
include system_wrappers/SOURCE.mk
include third_party/SOURCE.mk
include common_audio/SOURCE.mk
include modules/SOURCE.mk
include adapter_c/SOURCE.mk
# include demos/SOURCE.mk
srcs    := $(srcs_1) $(srcs_3) # $(srcs_2)
objs    := $(patsubst %.cc,%.o,$(filter %.cc, $(srcs))) \
           $(patsubst %.c,%.o,$(filter %.c, $(srcs))) \
           $(patsubst %.S,%.o,$(filter %.S, $(srcs)))
deps    := $(patsubst %.o,%.d,$(objs))
libs    := -lpthread
cflags   = -I. -DWEBRTC_POSIX # -DWEBRTC_WIN
cflags  += -Wno-unused-parameter -g # -DNDEBUG
ldflags := 
# for reproducible build
objs    := $(sort $(objs))
# cflags  += -Wno-builtin-macro-redefined -U__FILE__ -D__FILE__=\"$(notdir $<)\"

targets := lib$(name).so lib$(name).a
all : $(targets)

clean : 
	rm -f $(targets)
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
%.o : %.c
	@$(cc) -Os -Wall -Wextra -fPIC -std=c11 -D_POSIX_C_SOURCE=200809L $(cflags) -c $< -o $@ -MMD -MF $*.d -MP
	$(info $(cc) -c $(notdir $<) -o $(notdir $@))
%.o : %.S
	@$(cc) -Os -Wall -Wextra -fPIC -std=c11 -D_POSIX_C_SOURCE=200809L $(cflags) -c $< -o $@ -MMD -MF $*.d -MP
	$(info $(cc) -c $(notdir $<) -o $(notdir $@))

-include $(deps)
