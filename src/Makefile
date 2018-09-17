OUT=lexer.h lexer.re
IN=lexer.h.in lexer.re.in
TCLSH=$$(../autosetup/autosetup-find-tclsh)
all: $(OUT) .FORCE

$(OUT): $(IN)
	@echo script/preproc.tcl $(IN)
	@$(TCLSH) script/preproc.tcl $(IN)


clean: .FORCE
	rm -f $(OUT)
.FORCE:
