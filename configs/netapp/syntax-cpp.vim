" Highlight Class and Function names
syn match    cCustomParen    "?=(" contains=cParen,cCppParen
syn match    cCustomFunc     "\w\+\s*(\@=" contains=cCustomParen
syn match    cCustomScope    "::"
syn match    cCustomClass    "\w\+\s*::" contains=cCustomScope
syn match	 cCustomType /\w\+_t\w\@!/

hi cCustomFunc ctermfg=36
hi cCustomClass ctermfg=red
hi def link cCustomType Type

