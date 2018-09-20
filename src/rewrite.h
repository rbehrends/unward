#pragma once

SourceList *UpdateUnsafeSections(SectionList *sections, StrSet *filter);
SourceList *GenUnsafeCode(FuncList *funcs, StrSet *filter);
void RewriteSourceFiles(SourceList *sources, Str *outputdir);
