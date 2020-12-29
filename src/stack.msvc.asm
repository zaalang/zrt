;
; chkstk
;

.code
__chkstk PROC
      push rcx
	  push rax
      cmp rax, 1000h
      lea rcx, [rsp+16]
      jb A
   B: sub rcx, 1000h
      test [rcx], rcx
      sub rax, 1000h
      cmp rax, 1000h
      ja B
   A: sub rcx, rax
      test [rcx], rcx
      pop rax
      pop rcx
      ret
__chkstk ENDP
END
