################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
utils/moving_average_filter/test/%.obj: ../utils/moving_average_filter/test/%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv8/tools/compiler/ti-cgt-msp430_19.6.0.STS/bin/cl430" -vmspx --code_model=large --data_model=large --use_hw_mpy=F5 --include_path="C:/ti/ccsv8/ccs_base/msp430/include" --include_path="C:/Users/krad2/workspace_v8/ms-tos-refactor" --include_path="C:/ti/ccsv8/tools/compiler/ti-cgt-msp430_19.6.0.STS/include" --advice:power="all" --define=__MSP430F5529__ -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="utils/moving_average_filter/test/$(basename $(<F)).d_raw" --obj_directory="utils/moving_average_filter/test" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


