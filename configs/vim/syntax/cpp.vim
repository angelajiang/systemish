" Highlight Class and Function names
syn match    cCustomParen    "?=(" contains=cParen,cCppParen
syn match    cCustomFunc     "\w\+\s*(\@=" contains=cCustomParen
syn match    cCustomScope    "::"
syn match    cCustomClass    "\w\+\s*::" contains=cCustomScope

" Match class names beginning with a capital letter.
" 1. <\ ==> Word match begin
" 2. \u\+ ==> 1 or more uppercase letters
" 3. \l\+ ==> or more lowercase letters. This prevents us from matching all-caps
"    macros
" 4. \h* ==> 0 or more of {A-Z, a-z, _}
" 5. >/ ==> Word match end
syn match    cCustomClass2    /\<\u\+\l\+\h*\>/
syn match	 cCustomType /\w\+_t\w\@!/

hi cCustomFunc ctermfg=36
hi cCustomClass ctermfg=red
hi def link cCustomClass2 Type
hi def link cCustomType Type

