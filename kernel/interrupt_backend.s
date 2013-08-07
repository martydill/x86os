
; Entry points for all of our interrupts
; Each one pushes an error code (or a null value) onto the stack,
; then pushes the interrupt number, then calls _interrupt_handler,
; which fires KeExceptionHandler

extern KeExceptionHandler


global _interrupt0
_interrupt0:
	cli
	push 0
	push 0
	jmp _interrupt_handler

global _interrupt1
_interrupt1:
	cli
	push 0
	push 1
	jmp _interrupt_handler

global _interrupt2
_interrupt2:
	cli
	push 0
	push 2
	jmp _interrupt_handler

global _interrupt3
_interrupt3:
	cli
	push 0
	push 3
	jmp _interrupt_handler

global _interrupt4
_interrupt4:
	cli
	push 0
	push 4
	jmp _interrupt_handler

global _interrupt5
_interrupt5:
	cli
	push 0
	push 5
	jmp _interrupt_handler

global _interrupt6
_interrupt6:
	cli
	push 0
	push 6
	jmp _interrupt_handler

global _interrupt7
_interrupt7:
	cli
	push 0
	push 7
	jmp _interrupt_handler

global _interrupt8
_interrupt8:
	cli
	push 8
	jmp _interrupt_handler

global _interrupt9
_interrupt9:
	cli
	push 0
	push 9
	jmp _interrupt_handler

global _interrupt10
_interrupt10:
	cli
	push 10
	jmp _interrupt_handler

global _interrupt11
_interrupt11:
	cli
	push 11
	jmp _interrupt_handler

global _interrupt12
_interrupt12:
	cli
	push 12
	jmp _interrupt_handler

global _interrupt13
_interrupt13:
	cli
	push 13
	jmp _interrupt_handler

global _interrupt14
_interrupt14:
	cli
	push 14
	jmp _interrupt_handler

global _interrupt15
_interrupt15:
	cli
	push 0
	push 15
	jmp _interrupt_handler



global _interrupt32
_interrupt32:
	cli
	push 0
	push 32
	jmp _interrupt_handler

global _interrupt33
_interrupt33:
	cli
	push 0
	push 33
	jmp _interrupt_handler

global _interrupt34
_interrupt34:
	cli
	push 0
	push 34
	jmp _interrupt_handler

global _interrupt35
_interrupt35:
	cli
	push 0
	push 35
	jmp _interrupt_handler

global _interrupt36
_interrupt36:
	cli
	push 0
	push 36
	jmp _interrupt_handler

global _interrupt37
_interrupt37:
	cli
	push 0
	push 37
	jmp _interrupt_handler

global _interrupt38
_interrupt38:
	cli
	push 0
	push 38
	jmp _interrupt_handler

global _interrupt39
_interrupt39:
	cli
	push 0
	push 39
	jmp _interrupt_handler

global _interrupt40
_interrupt40:
	cli
	push 0
	push 40
	jmp _interrupt_handler

global _interrupt41
_interrupt41:
	cli
	push 0
	push 41
	jmp _interrupt_handler


global _interrupt112
_interrupt112:
	cli
	push 0
	push 112
	jmp _interrupt_handler


global _reservedexceptionhandler
_reservedexceptionhandler:
	cli
	push 0
	push 999
	jmp _interrupt_handler



_interrupt_handler:

	; Save our registers
	pusha
    push ds
    push es
    push fs
    push gs

	; Set up kernel data segment
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; Send our stack to the exception handler
	mov eax, esp
	push eax
	mov eax, KeExceptionHandler
	call eax
	pop eax

    ; Restore our registers
    pop gs
    pop fs
    pop es
    pop ds
    popa

    ; Done!
    add esp, 8
    iret
