#pragma once

SourceList *UpdateUnsafeSections(SectionList *sections, StrSet *filter);
SourceList *GenUnsafeCode(FuncList *funcs, StrSet *used, StrSet *skip);
Str *SafeName(Str *name);
Str *UnsafeName(Str *name);
void RewriteSourceFiles(SourceList *sources, Options *opts);
