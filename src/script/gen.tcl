proc emit {args} {
  foreach str $args {
    puts -nonewline $str
  }
}

proc capitalize {str} {
  return [string toupper [string range $str 0 0]][string range $str 1 end]
}

set keywords "auto break case char const continue default do double
  else enum extern float for goto if int long register return short
  signed sizeof static struct switch typedef union unsigned void
  volatile while"

set other "Ident Literal Op Semicolon LPar RPar LBrkt RBrkt LBrace
  RBrace WS EOL Comment BAD PPCond PPDef PPOther EOF"
