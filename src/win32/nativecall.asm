.code
	CallNativeFunc proc 
	
	PUSH	RBP
	MOV		RBP,RSP

	; ����ָ����RCX

	; ���ϲ���
	MOV		EAX, 1234

	LEAVE
	RET 
	CallNativeFunc endp
end