;	STUB.ASM	demonstrates segment controls needed
;			to link MASM code to C88 with -M option.
;			Called with hello();.

;	notes: if you wish to start with an assembler module, there
;	       must be a public entry point of MAIN_ in the PROG segment. This
;	       is the starting point.

;	       PUBLIC and EXTRN statements should be in the same segment
;	       as the item is actually defined in.

;	       To communicate with a C program or the libraries, the
;	       GROUP and SEGMENT statements should be exactly as described.
;	       Remember that the supplied C.OBJ must be LINKed
;	       first. This is needed both to set up a stack and to
;	       establish the correct groups.

;	       The offset operator should always be followed by the
;	       GROUP name or you are likely to end up with the offset
;	       within the segment and not the group. See the example below.

DGROUP	GROUP	DATA,UDATA

PGROUP	GROUP	PROG

PROG	SEGMENT BYTE PUBLIC 'PROG'
	ASSUME	CS:PGROUP,DS:DGROUP

	PUBLIC	HELLO			;NOTE: ALTHOUGH C88 USES A TRAILING
					;UNDERSCORE ON PUBLICS, THESE ARE
					;DELETED IN .OBJ FORMAT.
	EXTRN	PUTS:NEAR

HELLO:	MOV	AX,OFFSET DGROUP:HELLO_MSG
	PUSH	AX
	CALL	PUTS
	POP	AX
	RET
PROG	ENDS



DATA	SEGMENT	WORD PUBLIC 'DATA'
HELLO_MSG	DB	'Hello World!!',13,10,0
DATA	ENDS

;	ALL DATA IN UDATA WILL BE ZEROED OUT AT RUN TIME.

UDATA	SEGMENT WORD PUBLIC 'UDATA'
BUFFER	DB	100 DUP (?)
UDATA	ENDS
	END
