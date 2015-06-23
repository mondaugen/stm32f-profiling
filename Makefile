CMSIS_INCLUDES=../../archives/CMSIS/Include
SRCS=$(wildcard src/*.c)
BIN=main.elf
OCD 		   = openocd -f /usr/share/openocd/scripts/board/stm32f4discovery.cfg
OPTIMIZE=-Ofast

$(BIN) : $(SRCS)
	arm-none-eabi-gcc -DSTM32F429_439xx $^ -Iinc -I$(CMSIS_INCLUDES) \
		-Tstm32f429.ld -o $@ -ggdb3 -mlittle-endian -mthumb -mcpu=cortex-m4 \
		-mthumb-interwork -mfloat-abi=hard -mfpu=fpv4-sp-d16 -dD -lm \
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
