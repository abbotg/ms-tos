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
            .cdecls C, LIST, "msp430.h"       	; Include device header file

            .text                           	; Assemble into program memory.
            .align 2

            .global preempt_reset
            .global schedule
            .global run_ptr
            .global num_ctx_switches

            .global preempt_firstrun

            .global isr_time_start
            .global isr_time_stop

;-------------------------------------------------------------------------------
; void context_save(void *)
;-------------------------------------------------------------------------------

context_save: .macro R4, R5, R6, R7, R8, R9, R10, SP, R12
			.if $DEFINED(__LARGE_CODE_MODEL__) | $DEFINED(__LARGE_DATA_MODEL__)
				mova   	R4,   0(R12)
	            mova   	R5,   4(R12)
	            mova   	R6,   8(R12)
	            mova  	R7,   12(R12)
	            mova   	R8,   16(R12)
	            mova   	R9,  20(R12)
	            mova   	R10, 24(R12)
	            mova   	SP,  28(R12)

	            mov.w	0(SP), 32(R12)			; save SR
	            mov.w  	2(SP), 34(R12)			; save PC
			.else
				mov.w   R4,   0(R12)
	            mov.w   R5,   2(R12)
	            mov.w   R6,   4(R12)
	            mov.w   R7,   6(R12)
	            mov.w   R8,   8(R12)
	            mov.w   R9,  10(R12)
	            mov.w   R10, 12(R12)
	            mov.w   SP,  14(R12)

	            mov.w	0(SP), 16(R12)			; save SR
	            mov.w   2(SP), 18(R12)			; save PC
	        .endif
            .endm

;-------------------------------------------------------------------------------
; void context_load(void *)
;-------------------------------------------------------------------------------
context_load: .macro R4, R5, R6, R7, R8, R9, R10, SP, R12
			.if $DEFINED(__LARGE_CODE_MODEL__) | $DEFINED(__LARGE_DATA_MODEL__)
				mova   0(R12), R4
	            mova   4(R12), R5
	            mova   8(R12), R6
	            mova   12(R12), R7
	            mova   16(R12), R8
	            mova   20(R12), R9
	            mova   24(R12), R10
	            mova   28(R12), SP
			.else
	            mov.w   0(R12), R4
	            mov.w   2(R12), R5
	            mov.w   4(R12), R6
	            mov.w   6(R12), R7
	            mov.w   8(R12), R8
	            mov.w   10(R12), R9
	            mov.w   12(R12), R10
	            mov.w   14(R12), SP
			.endif
            .endm

;-------------------------------------------------------------------------------
; Sets up interrupt stack for future return
;-------------------------------------------------------------------------------
context_load_epilogue: .macro SP, R12
			.if $DEFINED(__LARGE_CODE_MODEL__) | $DEFINED(__LARGE_DATA_MODEL__)
				mov.w	32(R12), 0(SP)										; Put SR on the stack for RETI
           	 	mov.w  	34(R12), 2(SP)          							; Put the PC on the stack for RETI
			.else
            	mov.w	16(R12), 0(SP)										; Put SR on the stack for RETI
           	 	mov.w   18(R12), 2(SP)          							; Put the PC on the stack for RETI
			.endif
            .endm

WDT_ISR:
			.if $DEFINED(__LARGE_CODE_MODEL__) | $DEFINED(__LARGE_DATA_MODEL__)
				mova	&run_ptr, R12
			.else
           	 	mov.w   &run_ptr, R12           							; Get running TCB context
            .endif

			context_save R4, R5, R6, R7, R8, R9, R10, SP, R12				; Save context to current TCB

			.if $DEFINED(__LARGE_CODE_MODEL__) | $DEFINED(__LARGE_DATA_MODEL__)
				calla #schedule
				calla #preempt_reset										; Reset scheduler timer
			.else
				call    #schedule               							; Update run_ptr
            	call    #preempt_reset          							; Reset scheduler timer
			.endif


            .if $DEFINED(__LARGE_CODE_MODEL__) | $DEFINED(__LARGE_DATA_MODEL__)
				mova	&run_ptr, R12
			.else
           	 	mov.w   &run_ptr, R12           							; Get new TCB context to run
            .endif

			context_load R4, R5, R6, R7, R8, R9, R10, SP, R12				; Load new TCB context
			context_load_epilogue SP, R12									; Prepare interrupt stack for RETI
            reti


preempt_firstrun: .asmfunc
            .if $DEFINED(__LARGE_CODE_MODEL__) | $DEFINED(__LARGE_DATA_MODEL__)
				mova	&run_ptr, R12
			.else
           	 	mov.w   &run_ptr, R12           							; Get new TCB context to run
            .endif

			context_load R4, R5, R6, R7, R8, R9, R10, SP, R12				; Load context

			.if $DEFINED(__LARGE_CODE_MODEL__) | $DEFINED(__LARGE_DATA_MODEL__)
				calla #preempt_reset										; Reset scheduler timer
			.else
				call #preempt_reset
			.endif

            nop
            eint                            								; Enable interrupts for scheduler operation
            nop

			.if $DEFINED(__LARGE_CODE_MODEL__) | $DEFINED(__LARGE_DATA_MODEL__)
				bra      36(R12)
			.else
				br      18(R12)												; Branch to first task
			.endif

            .endasmfunc

            ; WDT ISR
			.intvec WDT_VECTOR, WDT_ISR
            .end
