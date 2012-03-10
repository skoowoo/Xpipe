/**
 * Copyright (c) Xiaowei Wu
 */

#ifndef __XSTRING_H__
#define __XSTRING_H__

/**
 * len is the string's lengh, it don't contain '\0'. data point to a string 
 * which have '\0'. 
 */
typedef struct {
    size_t     len;
    u_char    *data;
} string_t;


#define xstring(str)   { sizeof(str) - 1, (u_char *) str }
#define set_string(str, text)  \
   (str)->len = sizeof(text) - 1; (str)->data = (u_char *) text 

#define string_null { 0, NULL }

#define x_tolower(c)  (u_char) ((c >= 'A' && c <= 'Z') ? (c | 0x20) : c)
#define x_toupper(c)  (u_char) ((c >= 'a' && c <= 'z') ? (c & ~0x20) : c)

#define is_letter(c)  ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))  
#define is_digit(c)  (c >= '0' && c <= '9')
#define is_blank(c)  (c == CR || c == LF || c == '\t' || c == ' ')

#define x_memcpy_n(dst, src, n) ((u_char *)memcpy(dst, src, n) + n)

#define x_strcpy(s1, s2)  strcpy((char *) (s1), (char *) (s2))
#define x_strcmp(s1, s2)  strcmp((const char *) (s1), (const char *) (s2))
#define x_strncmp(s1, s2, n)  \
    strncmp((const char *) (s1), (const char *) (s2), n)
#define x_strlen(s)  strlen((const char *) s)
#define x_atoi(s) atoi((const char *) s)

#define is_full_file_name(file_name) \
        (file_name->len != 0 && *(file_name->data) == '/')

int is_positive_integer(string_t *num);
void make_full_file_name(u_char *full_name, u_char *name, int len);

#endif  /* __STRING_H__ */
