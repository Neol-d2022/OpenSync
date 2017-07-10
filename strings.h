#ifndef STRINGS_H_INCLUDED
#define STRINGS_H_INCLUDED

char *StringsRemoveNewline(char *str);
char **StringsSplit(const char *str, int ch, unsigned int *count);

#endif
