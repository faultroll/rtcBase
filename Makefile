.POSIX :

# dtop := ../../..
dtmp := ./tmp
dtoolsrc := ./tool
dtooldst := $(dtmp)/buildtool
crossfile := $(dtmp)/cross-file.txt

# include $(dtop)/platform.conf
PRFX ?= 
ARCH ?= x86_64
ddist := ../dist/$(CHIP_TYPE)

.PHONY : all clean demo_all demo_clean
all : demo_all $(ddist)/libwebrtc_audio.a $(ddist)/libwebrtc_audio_adapter.a

clean : demo_clean
	@rm -f $(ddist)/libwebrtc_audio_adapter.a
	@rm -f ./config.h $(ddist)/libwebrtc_audio.a
	@rm -rf $(dtmp)

demo_all : $(ddist)/libwebrtc_audio.a $(ddist)/libwebrtc_audio_adapter.a
	@$(PRFX)g++ -std=c++11 -I. $(wildcard demo/Mix/*.cpp) -o demoMix $(ddist)/libwebrtc_audio.a -lpthread
	@$(PRFX)g++ -std=c++11 -I. $(wildcard demo/ConferenceMix/*.cpp) -o demoConferenceMix $(ddist)/libwebrtc_audio.a -lpthread
#	@$(PRFX)g++ -std=c++11 -I. $(wildcard demo/Noise/*.cpp) -o demoNoise $(ddist)/libwebrtc_audio.a -lpthread
#	@$(PRFX)g++ -std=c++11 -I. $(wildcard demo/Echo/*.cpp) -o demoEcho $(ddist)/libwebrtc_audio.a -lpthread
	@$(PRFX)g++ -std=c++11 -I. $(wildcard demo/NetEq/*.cpp) -o demoNetEq $(ddist)/libwebrtc_audio.a -lpthread
	@$(PRFX)gcc -std=c11 -I. $(wildcard demo/Aproc/*.c) -o demoAproc $(ddist)/libwebrtc_audio_adapter.a $(ddist)/libwebrtc_audio.a -lpthread -lstdc++

demo_clean :
	@rm -f demoMix
	@rm -f demoConferenceMix
#	@rm -f demoNoise
#	@rm -f demoEcho
	@rm -f demoNetEq
	@rm -f demoAproc


$(ddist)/libwebrtc_audio.a : $(dtmp)
	@PATH="${PATH}:$(dtooldst)" $(dtooldst)/meson.py $(dtmp) --cross-file $(crossfile)
	@$(dtooldst)/ninja -C $(dtmp)
	@cp -f $(dtmp)/config.h ./ && cp -f $(dtmp)/libwebrtc_audio.a $(ddist)/

# TODO no need to deploy each time
$(dtmp) :
	@rm -rf $(dtmp) # && rm -rf $(dtooldst)
	@mkdir -p $(dtmp)
	@tar -xzf $(dtoolsrc)/meson-0.54.3.tar.gz -C $(dtmp) && mv $(dtmp)/meson-0.54.3 $(dtooldst)
	@unzip -qo $(dtoolsrc)/ninja-linux.zip -d $(dtmp) && mv $(dtmp)/ninja $(dtooldst)/
	@chmod +x $(dtooldst)/* -R
	@ \
	echo "[binaries]" > $(crossfile) && \
	echo "c = '$(PRFX)gcc'" >> $(crossfile) && \
	echo "cpp = '$(PRFX)g++'" >> $(crossfile) && \
	echo "ar = '$(PRFX)ar'" >> $(crossfile) && \
	echo "strip = '$(PRFX)strip'" >> $(crossfile) && \
	echo "pkgconfig = '/usr/bin/pkg-config'" >> $(crossfile) && \
	echo "[properties]" >> $(crossfile) && \
	echo "pkg_config_libdir = ''" >> $(crossfile) && \
	echo "[host_machine]" >> $(crossfile) && \
	echo "system = 'linux'" >> $(crossfile) && \
	echo "cpu_family = '$(ARCH)'" >> $(crossfile) && \
	echo "cpu = '$(ARCH)'" >> $(crossfile) && \
	echo "endian = 'little'" >> $(crossfile)

ada_objs := $(patsubst %.cpp,%.o,$(wildcard adapter/*.cpp)) 
$(ddist)/libwebrtc_audio_adapter.a : $(ada_objs)
	@$(PRFX)ar -crD $@ $^
	@$(PRFX)ranlib -D $@

%.o : %.cpp
	@$(PRFX)g++ -std=c++11 -Os -fPIC -I. -I$(dtop)/common -c $< -o $@
	$(info $(PRFX)gcc -c $(notdir $<) -o $(notdir $@))

$(ada_objs) : $(ddist)/libwebrtc_audio.a
