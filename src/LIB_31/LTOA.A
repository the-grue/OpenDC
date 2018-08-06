;
;  Released under the GNU LGPL.  See http://www.gnu.org/licenses/lgpl.txt
;
;  This program is part of the DeSmet C Compiler
;
;  This library is free software; you can redistribute it and/or modify
;  it under the terms of the GNU Lesser General Public License as published
;  by the Free Software Foundatation; either version 2.1 of the License, or
;  any later version.
;
;  This library is distributed in the hope that it will be useful, but
;  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
;  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
;  License for more details.
;
 CSEG 
 PUBLIC ltoa_

ltoa_: PUSH BP
 MOV BP,SP
 SUB SP,10
 MOV AX,WORD [BP+8]
 MOV WORD [BP-4],AX
 MOV WORD [BP-6],AX
 MOV WORD [BP-10],0
 CMP WORD [BP+10],2
 JL _L2
 CMP WORD [BP+10],36
 JLE _L1
_L2:
 MOV SI,WORD [BP+8]
 MOV AX,SI
 MOV BYTE [SI],0
 MOV SP,BP
 POP BP
 RET
_L1:
 CMP WORD [BP+10],10
 JNZ _L3
 TEST BYTE [BP+7],80H
 JZ _L3
 INC WORD [BP-10]
 NOT WORD [BP+6]
 NEG WORD [BP+4]
 SBB WORD [BP+6],-1
_L3:
 MOV AX,WORD [BP+8]
 MOV WORD [BP-6],AX
_L4:
 MOV CX,WORD [BP+10]
 XOR BX,BX
 MOV DX,WORD [BP+6]
 MOV AX,WORD [BP+4]
 PUBLIC _UMOD4
 CALL _UMOD4
 MOV WORD [BP-8],AX
 CMP WORD [BP-8],10
 JL _L7
 ADD AX,55
 JMP _L8
_L7:
 ADD AX,48
_L8:
 MOV SI,WORD [BP-6]
 INC WORD [BP-6]
 MOV BYTE [SI],AL
_L5:
 MOV CX,WORD [BP+10]
 XOR BX,BX
 MOV DX,WORD [BP+6]
 MOV AX,WORD [BP+4]
 PUBLIC _UDIV4
 CALL _UDIV4
 MOV WORD [BP+4],AX
 MOV WORD [BP+6],DX
 OR AX,DX
 JNZ _L4
_L6:
 MOV SI,WORD [BP-6]
 CMP WORD [BP-10],0
 JZ _L9
 MOV BYTE [SI],45
 INC SI
 INC WORD [BP-6]
_L9:
 MOV BYTE [SI],0
_L10:
 MOV AX,WORD [BP-6]
 CMP AX,WORD [BP+8]
 JBE _L11
 MOV SI,WORD [BP+8]
 MOV AL,BYTE [SI]
 MOV BYTE [BP-1],AL
 DEC WORD [BP-6]
 MOV SI,WORD [BP-6]
 MOV DI,WORD [BP+8]
 INC WORD [BP+8]
 MOV AL,BYTE [SI]
 MOV BYTE [DI],AL
 MOV SI,WORD [BP-6]
 MOV AL,BYTE [BP-1]
 MOV BYTE [SI],AL
 JMP _L10
_L11:
 MOV AX,WORD [BP-4]
 MOV SP,BP
 POP BP
 RET
 END
