;-------------------------------------------------------------------------------
; Author: gta
;    
;  Original code borrowed from krad246
;  https://github.com/krad246/scheduler
;
; MSP430/MSP430X CALLING CONVENTION
;  Callee-saved: SP, R4-R10
;  Caller-saved: R11-R15
;  Function args: R12-R15
;
;-------------------------------------------------------------------------------
            .cdecls "defines.h"


            .sect ".text:_isr"
            .align 2

            .global preempt_reset
            .global schedule
            .global run_ptr
            .global num_ctx_switches

            .global preempt_firstrun

            .global isr_time_start
            .global isr_time_stop

            .global context_switcher

;-------------------------------------------------------------------------------
; void context_save(void *)
;-------------------------------------------------------------------------------

context_save: .macro R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, SP, R15
			.if $DEFINED(RTOS_20BIT__)			; Save registers
				mova   	R4,   0(R15)
	            mova   	R5,   4(R15)
	            mova   	R6,   8(R15)
	            mova  	R7,   12(R15)
	            mova   	R8,   16(R15)
	            mova   	R9,  20(R15)
	            mova   	R10, 24(R15)
	            mova   	R11, 28(R15)
	            mova   	R12, 32(R15)
	            mova   	R13, 36(R15)
	            mova   	R14, 40(R15)
	            mova   	SP,  44(R15)

	            mov.w	4(SP), 48(R15)			; Copy trapframe over to context block
	            mov.w  	6(SP), 50(R15)
			.else
				mov.w   R4,   0(R15)
	            mov.w   R5,   2(R15)
	            mov.w   R6,   4(R15)
	            mov.w   R7,   6(R15)
	            mov.w   R8,   8(R15)
	            mov.w   R9,  10(R15)
	            mov.w   R10, 12(R15)
	           	mov.w   R11, 14(R15)
	            mov.w   R12, 16(R15)
	            mov.w   R13, 18(R15)
	            mov.w   R14, 20(R15)
	            mov.w   SP,  22(R15)

	            mov.w	2(SP), 24(R15)			; save SR
	            mov.w   4(SP), 26(R15)			; save PC
	        .endif
            .endm

;-------------------------------------------------------------------------------
; void context_load(void *)
;-------------------------------------------------------------------------------
context_load: .macro R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, SP, R15
			.if $DEFINED(RTOS_20BIT__)
				mova   0(R15), R4
	            mova   4(R15), R5
	            mova   8(R15), R6
	            mova   12(R15), R7
	            mova   16(R15), R8
	            mova   20(R15), R9
	            mova   24(R15), R10
	            mova   28(R15), R11
	            mova   32(R15), R12
	            mova   36(R15), R13
	            mova   40(R15), R14
	            mova   44(R15), SP
			.else
	            mov.w   0(R15), R4
	            mov.w   2(R15), R5
	            mov.w   4(R15), R6
	            mov.w   6(R15), R7
	            mov.w   8(R15), R8
	            mov.w   10(R15), R9
	            mov.w   12(R15), R10
	            mov.w   24(R15), R11
	            mov.w   16(R15), R12
	            mov.w   18(R15), R13
	            mov.w   20(R15), R14
	            mov.w   22(R15), SP
			.endif
            .endm

;-------------------------------------------------------------------------------
; Sets up interrupt stack for future return
;-------------------------------------------------------------------------------
context_load_epilogue: .macro SP, R15
			.if $DEFINED(RTOS_20BIT__)
				mov.w	48(R15), 4(SP)															; Put SR on the stack for RETI
           	 	mov.w  	50(R15), 6(SP)          												; Put the PC on the stack for RETI
			.else
            	mov.w	24(R15), 2(SP)															; Put SR on the stack for RETI
           	 	mov.w   26(R15), 4(SP)          												; Put the PC on the stack for RETI
			.endif
            .endm

context_switcher:
			.if $DEFINED(RTOS_20BIT__)
				pushx.a R15																		; Save scratch register R15

				mova	&run_ptr, R15															; Fetch current task (overwrites R15)
				context_save R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, SP, R15			; Save context to current TCB

				calla #schedule																	; Find next TCB to run
				calla #preempt_reset															; Reset scheduler timer

				mova	&run_ptr, R15															; Fetch new calculated TCB
				context_load R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, SP, R15			; Load new TCB context
				context_load_epilogue SP, R15													; Prepare interrupt stack for RETI

				popx.a R15																		; Reload scratch register (don't need run pointer anymore)
			.else
				push.w  R15

           	 	mov.w   &run_ptr, R15
            	context_save R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, SP, R15

            	call    #schedule
            	call    #preempt_reset

            	mov.w   &run_ptr, R15
            	context_load R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, SP, R15
				context_load_epilogue SP, R15
				pop.w R15
            .endif

            reti																				; Branch to task


preempt_firstrun: .asmfunc
            .if $DEFINED(RTOS_20BIT__)
				mova	&run_ptr, R15
				context_load R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, SP, R15			; Load context

				calla #preempt_reset															; Reset scheduler timer

				popx.a R15																		; Dummy variable used in subsequent calls, needs to be popped
			.else
           	 	mov.w   &run_ptr, R15
           	 	context_load R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, SP, R15

           	 	call #preempt_reset

				pop.w R15
            .endif

				reti																			; Branch to first task

            .endasmfunc

			.sect WDT_VECTOR
			.word context_switcher
			.end
