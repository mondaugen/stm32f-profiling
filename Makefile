CMSIS_INCLUDES=../../archives/CMSIS/Include
SRCS=$(wildcard src/*.c)
BIN=main.elf
OCD 		   = openocd -f /usr/share/openocd/scripts/board/stm32f4discovery.cfg
OPTIMIZE=-Ofast
INCPATHS=inc ../mm_dsp/inc ../mm_primitives/inc ../ne_datastructures/inc
LIBPATHS=../mm_dsp/lib ../mm_primitives/lib ../ne_datastructures/lib
DEP=$(foreach inc,$(INCPATHS), $(wildcard $(inc)/*.h))
DEP+=$(foreach lib,$(LIBPATHS), $(wildcard $(lib)/*.a))
LIB=$(foreach lib,$(LIBPATHS),-L$(lib))
INC=$(foreach inc,$(INCPATHS),-I$(inc))

$(BIN) : $(SRCS) $(DEP)
	arm-none-eabi-gcc -DSTM32F429_439xx $(filter %.c %.s, $^) $(INC) \
		-I$(CMSIS_INCLUDES) $(LIB) -Wall \
		-Tstm32f429.ld -o $@ -ggdb3 -mlittle-endian -mthumb -mcpu=cortex-m4 \
		-mthumb-interwork -mfloat-abi=hard -mfpu=fpv4-sp-d16 -dD -lm \
		-lmm_dsp -lmm_primitives -lne_datastructures \
		--specs=nano.specs $(OPTIMIZE)

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
	ctags -R . $(CMSIS_INCLUDES)
