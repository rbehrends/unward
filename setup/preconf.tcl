add-program unward src
add-makefile-var "TCLSH" "`autosetup/autosetup-find-tclsh`"
add-makefile-var "PREPROC_TCL" "\$(TCLSH) src/script/preproc.tcl"
add-makefile-var "RE2CFLAGS" "--no-generation-date --no-version"
add-dependency "src/lexer.h" "src/lexer.h.in" \
  "\$(PREPROC_TCL) src/lexer.h.in"
add-dependency "src/lexer.re" "src/lexer.h.in" \
  "\$(PREPROC_TCL) src/lexer.re.in"
