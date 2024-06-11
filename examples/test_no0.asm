
[addr=0x00, align=2, size=64, perm=rx]
vecs:
	reset
	led-on
end


[addr=auto, align=2, size=auto, perm=rx]
reset:
	; ... 
	ret
end


[addr=auto, align=2, size=auto, perm=rx]
led-on:
	; ... 
	ret
end


[entry, addr=auto, align=2, size=auto, perm=rx]
start:
	movw r0, 1
	movw r1, 1
	stw is-running, 0x01  ; store value True into the variable is-running.
	.loop:
		cmp is-running, 0x01  ; if is-running is True.
		bne .loop-end         ; jump to the end of the program if is-running is False.
		; do the application logic stuff...
		br .loop  ; jump back to the loop condition to check if is-running is still True.
	.loop-end:    ; end of the program.
	; ... 
	ret
end


[addr=0x1000, align=2, size=1, perm=rw]
is-running:
end


#define STACK_BEGIN 0x2000
#define STACK_SIZE  0x200


[addr=STACK_BEGIN, align=2, size=STACK_SIZE, perm=rw]
stack:
end
