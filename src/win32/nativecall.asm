.code
	CallNativeFunc proc 
	
	push	rbp
	mov		rbp,rsp

	; ����ָ����rcx
	; ������ַ��rdx
	; ����������r8
	; ����ָ����r9

	; ���ϲ���
	mov		rax,0
	mov		r10,rsp
	sub		r10,20h
LOOPBEGIN:
	cmp		rax,r8
	jae		LOOPEND

	; ѭ��copy����
	; type
	mov		rbx,qword ptr [rdx]
	mov		qword ptr [r10],rbx
	; data
	mov		rbx,qword ptr [rdx+8h]
	mov		qword ptr [r10+8h],rbx
	
	add		rdx,10h
	sub		r10,10h

	add		rax,1
	jmp		LOOPBEGIN

LOOPEND:
	call    rcx

	; ����ֵ
	mov		qword ptr [r9],rax
	mov		qword ptr [r9+8h],rdx

	leave
	ret 
	CallNativeFunc endp
end