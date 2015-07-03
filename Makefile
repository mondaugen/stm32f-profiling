CMSIS_INCLUDES=../../build/CMSIS/Include
CMSIS_MATHLIB=../../build/CMSIS/Lib/GCC
CMSIS_MATHSRC=../../build/CMSIS/DSP_Lib/Source/lib
SRCS=$(wildcard src/*.c)
BIN=main.elf
OCD 		   = openocd -f /usr/share/openocd/scripts/board/stm32f4discovery.cfg
PATHS   = ./ ../mm_dsp ../mm_primitives ../ne_datastructures
INCPATHS= $(foreach path,$(PATHS),$(path)/inc) $(CMSIS_INCLUDES)
LIBPATHS= $(foreach path,$(PATHS),$(path)/lib) $(CMSIS_MATHLIB)
DEP=$(foreach inc,$(INCPATHS), $(wildcard $(inc)/*.h))
DEP+=$(foreach lib,$(LIBPATHS), $(wildcard $(lib)/*.a))
LIB=$(foreach lib,$(LIBPATHS),-L$(lib))
INC=$(foreach inc,$(INCPATHS),-I$(inc))

OPTIMIZE=-Ofast

$(BIN) : $(SRCS) $(DEP)
	arm-none-eabi-gcc -DSTM32F429_439xx -DARM_MATH_CM4\
		$(filter %.c %.s, $^) $(INC) \
		 $(LIB) -Wall \
		-Tstm32f429.ld -o $@ -ggdb3 -mlittle-endian -mthumb -mcpu=cortex-m4 \
		-mthumb-interwork -mfloat-abi=hard -mfpu=fpv4-sp-d16 -dD -lm \
		-lmm_dsp -lmm_primitives -lne_datastructures -larm_cortexM4lf_math \
		--specs=nano.specs $(OPTIMIZE) -ffunction-sections

flash: $(BIN)
	$(OCD) -c init \
		-c "reset halt" \
	    -c "flash write_image erase $(BIN)" \
		-c "reset run" \
	    -c shutdown

reset: $(BIN)
	$(OCD) -c init -c "reset run" -c shutdown

clean:
	rm $(BIN)

tags:
	ctags -R . $(PATHS) $(CMSIS_INCLUDES) $(CMSIS_MATHSRC)
