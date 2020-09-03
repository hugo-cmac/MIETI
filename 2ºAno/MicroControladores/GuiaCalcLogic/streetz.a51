 #include		<at89C51ic2.h>

	CSEG 		AT 	0H
	JMP 		READY

	EXTRN 		CODE(CONFIG_RS232,ENVIAR_RS232,RECEBER_RS232)
	SCROLL 		EQU P3.5
	SETT 		EQU P3.3
	DSS 		EQU P1

	TABLE: 		DB 05H,0CH,0E5H,04H,15H,26H,0BCH,0CEH

	JUMP_TABLE:
			AJMP 		OP_ADD//0
			AJMP 		OP_OR//2
			AJMP 		OP_NOT//4
			AJMP 		OP_XOR//6
			AJMP 		OP_AND//8
			AJMP 		OP_SUB
			AJMP 		OP_ROTL
			AJMP 		OP_ROTR


	CSEG 		AT 50H

	READY:
			SETB		P1.2
			MOV 		DSS,#0CFH
			LCALL 		CONFIG_RS232
			L:
			JNB 		SETT,LEITURA
			JNB			SCROLL,ANTERIOR
			SJMP 		L

	ANTERIOR:
			JB			SCROLL,$
			JNB			SCROLL,$ 
			MOV 		A,R3
			MOV			R0,A
			LCALL		SEND
			CLR 		A
			JMP 		SELECT

	LEITURA:
			JB			SETT,$
			JNB			SETT,$ 
			MOV 		A,P2
			MOV			R3,A
			LCALL 		SEND
			MOV 		R0,P2
			CLR			cy
			SJMP 		SELECT

	SETUP:
			MOV 		DPTR, #TABLE
			MOVC 		A, @A+DPTR
			MOV 		DSS,A
			RET

	SELECT:
			MOV			A,R2
			CALL 		SETUP
			INC			R2
			CJNE		R2,#08H,BOTAO
			MOV			R2,#00H
			SJMP 		SELECT

	BOTAO:
			JNB 		SETT, SETUP1
			JNB 		SCROLL,L5
			SJMP		BOTAO
			L5:
			JB			SCROLL,$
			JNB			SCROLL,$
			JMP SELECT 

	OP_AND:
			LCALL 	OP2
			MOV 		A, R0
			ANL 		A, R1
			LJMP 		PROCESS

	OP_OR:
			LCALL 	OP2
			MOV 		A, R0
			ORL 		A, R1
			LJMP 		PROCESS

	OP_NOT:
			MOV 		A, R0
			CPL 		A
			LJMP 		PROCESS

	OP_XOR:
			LCALL 	OP2
			MOV 		A, R0
			XRL 		A,R1
			LJMP 		PROCESS

	OP_ADD:
			LCALL 	OP2
			MOV 		A, R0
			ADD 		A, R1
			JB 			cy,OVERFLOW
			LJMP 		PROCESS

	OP_SUB:
			LCALL 	OP2
			MOV 		A, R0
			SUBB 		A, R1
			JB 			cy,OVERFLOW
			LJMP 		PROCESS

	OP_ROTL:
			MOV 		A, R0
			RL 			A
			JB 			cy,OVERFLOW
			LJMP 		PROCESS

	OP_ROTR:
			MOV 		A, R0
			RR 			A
			JB 			cy,OVERFLOW
			LJMP 		PROCESS

	OVERFLOW:
			CLR			P1.2
			LJMP 		PROCESS

	SETUP1:
			JB 			SETT,$
			JNB 		SETT,$
			MOV 		DPTR, #JUMP_TABLE
			CALL		OP_SELECT
			CLR			cy
			JMP 		@A+DPTR

	OP_SELECT:
			DEC			R2
			MOV			A,R2
			JZ			L3
			CLR			A
			L2:
			ADD			A,#02H
			DJNZ		R2,L2
			RET
			L3:
			RET

	OP2:
			MOV			DSS,#054H 
			JNB			SETT,L8
			SJMP 		OP2
			L8:
			JB 			SETT,$
			JNB 		SETT,$
			MOV 		A,P2
			MOV 		R1,A
			MOV			R3,A
			LCALL		SEND
			CLR			A
			RET

	PROCESS:
			MOV			DSS,#046H
			MOV			R3,A
			CALL 		SEND
			L6:
			JNB			SETT,L7
			SJMP 		L6
			L7:
			JB 			SETT,$
			JNB 		SETT,$
			LJMP 		READY
	SEND:
			MOV			A,R3
			ANL			A,#0F0H
			SWAP		A
			CALL		HEX
			LCALL 		ENVIAR_RS232
			MOV			A,R3
			ANL			A,#00FH
			CALL		HEX
			LCALL 		ENVIAR_RS232
			RET


	HEX:
			CJNE		A,#00AH,H1
			MOV			A,#041H
			RET
			H1:
			CJNE		A,#00BH,H2
			MOV			A,#042H
			RET
			H2:
			CJNE		A,#00CH,H3
			MOV			A,#043H
			RET
			H3:
			CJNE		A,#00DH,H4
			MOV			A,#044H
			RET
			H4:
			CJNE		A,#00EH,H5
			MOV			A,#045H
			RET
			H5:
			CJNE		A,#00FH,H6
			MOV			A,#046H
			RET
			H6:
			ADD			A,#30H
			RET

END