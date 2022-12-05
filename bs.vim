
if exists("b:current_syntax")
  finish
endif

syn keyword bsKeyword let return if else while do for goto arr extern include_asm
syn keyword bsRegister A X Y

syn match bsDoubleQuoteEscape /\\[\\"$\n]/ contained
syn region bsString start=/"/ skip=/\v(\\{2})|(\\)"/ end=/"/ contains=bsDoubleQuoteEscape

syn match bsQuoteEscape /\\[\\'$\n]/ contained
syn region bsString start=/'/ skip=/\v(\\{2})|(\\)"/ end=/'/ contains=bsQuoteEscape



syn match bsDirective "#[^ \t\n]*"
syn match bsInclude "#include[ \t]*\".*\""
syn match bsSysInclude "#include[ \t]*<.*>"

syn match bsOperator '[\=*/~%&|<>!+-]'

syn match bsIdentifier "[a-zA-Z_][a-zA-Z0-9_]*"
syn match bsLabel "[a-zA-Z_][a-zA-Z_0-9]*:"

syn match bsDecimal "[0-9][0-9]*"
syn match bsHexidecimal "0[xX][0-9a-fA-F][0-9a-fA-F]*"
syn match bsBinary "0[bB][01][01]*"

syn include @Spdr syntax/spdr.vim
syn region bsInlineAsm matchgroup=Snip start="``" end="``" contains=@Spdr

syn match bsComment "//.*"
syn region bsComment start="/\*" end="\*/"

hi def link bsKeyword Keyword
hi def link bsDoubleQuoteEscape String
hi def link bsString String

hi def link bsQuoteEscape String
hi def link bsChar String

hi def link bsInclude Special
hi def link bsSysInclude Special
hi def link bsDirective	Special
hi def link bsOperator Operator
hi def link bsRegister Operator

hi def link bsLabel Label

hi def link bsChar String
hi def link bsDecimal Number
hi def link bsHexidecimal Number
hi def link bsBinary Number
hi def link bsComment Comment

let b:current_syntax = 'bs'

