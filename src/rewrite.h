#pragma once

SourceList *UpdateUnsafeSections(SectionList *sections, StrSet *filter);
SourceList *GenUnsafeCode(FuncList *funcs, StrSet *filter,
  StrSet *dont_rewrite);
void RewriteSourceFiles(SourceList *sources, Str *outputdir);
