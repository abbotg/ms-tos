################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: GNU Compiler'
	"C:/ti/ccsv8/tools/compiler/msp430-gcc-8.3.0.16_win32/bin/msp430-elf-gcc.exe" -c -mmcu=msp430f5529 -mhwmult=f5series -I"C:/ti/ccsv8/ccs_base/msp430/include_gcc" -I"C:/Users/krad2/workspace_v8/ms-tos" -I"C:/ti/ccsv8/tools/compiler/msp430-gcc-8.3.0.16_win32/msp430-elf/include" -Og -g -gdwarf-3 -gstrict-dwarf -Wall -mlarge -minrt -mcode-region=none -mdata-region=none -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

%.o: ../%.s $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: GNU Compiler'
	"C:/ti/ccsv8/tools/compiler/msp430-gcc-8.3.0.16_win32/bin/msp430-elf-gcc.exe" -c -mmcu=msp430f5529 -mhwmult=f5series -I"C:/ti/ccsv8/ccs_base/msp430/include_gcc" -I"C:/Users/krad2/workspace_v8/ms-tos" -I"C:/ti/ccsv8/tools/compiler/msp430-gcc-8.3.0.16_win32/msp430-elf/include" -Og -g -gdwarf-3 -gstrict-dwarf -Wall -mlarge -minrt -mcode-region=none -mdata-region=none -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -x assembler-with-cpp $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


