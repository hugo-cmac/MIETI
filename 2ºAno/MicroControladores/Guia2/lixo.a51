#include		<at89C51ic2.h>
#define			NLOOPS	4

	CSEG	AT	0H
	LJMP		MAIN
	
	CSEG	AT	50H
MAIN:
	MOV		20H,#1
	MOV		21H,#2
	MOV		22H,#3
	MOV		23H,#4
	
	MOV		R3,#NLOOPS
	MOV		R0,#20H
	MOV		R1,#30H
	
LOOP:
	CALL 	SubRotina
	DJNZ	R3,LOOP
	
/*LOOP:
	MOV		A,@R0
	MOV		@R1,A
	CPL		A
	MOV		P1,A
	INC		R0
	INC		R1
	DJNZ	R3,LOOP*/
	
AQUI:
	SJMP	AQUI
	
SubRotina:

	MOV		61H,#00
	MOV 	60H,#50H
	PUSH	60H
	PUSH	61H

	MOV		A,@R0
	MOV		@R1,A
	CPL		A
	MOV		P1,A
	INC		R0
	INC		R1
	POP		61H
	POP		60H
	//CALL SubRotina1
	RET
	
/*SubRotina1:
	MOV		A,@R0
	MOV		@R1,A
	CPL		A
	MOV		P1,A
	INC		R0
	INC		R1
	RET*/
DISPLAY:
	
	RET
	
END
	

//#include		<at89C51ic2.h>
	/*CSEG	AT	0H
	LJMP		MAIN
	
	CSEG	AT	50H*/
/*MAIN1:
	MOV		R0,#30H
	MOV		A,@R0
	MOV		B,#2
	DIV		AB
	JZ		
	
END*/
	
	