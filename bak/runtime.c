#define getchar getchar_
#include <stdio.h>
#include <stdlib.h>
#undef getchar

#define SAVE \
    unsigned int old_ebx, old_ecx, old_edx; \
    __asm__ volatile ("\t movl %%ebx,%0" : "=r"(old_ebx)); \
    __asm__ volatile ("\t movl %%ecx,%0" : "=r"(old_ecx)); \
    __asm__ volatile ("\t movl %%edx,%0" : "=r"(old_edx)); \

#define RESTORE \
    __asm__ volatile ("\t movl %0, %%ebx" :: "r"(old_ebx)); \
    __asm__ volatile ("\t movl %0, %%ecx" :: "r"(old_ecx)); \
    __asm__ volatile ("\t movl %0, %%edx" :: "r"(old_edx)); \

int *initArray(int size, int init)
{
    SAVE;
    //printf("ARRAY [%d] of %d\n", size, init);
    int i;
    //size++; // Store length in first slot
    int *a = (int *)malloc(size*sizeof(int));
    for(i=0;i<size;i++) a[i]=init;
    RESTORE;
    return a;
}

int *allocRecord(int size)
{
    SAVE;

    int i;
    int *p, *a;
    p = a = (int *)malloc(size);
    for(i=0;i<size;i+=sizeof(int)) *p++ = 0;
   
    RESTORE;

    return a;
}

struct string {int length; unsigned char chars[1];};

int stringEqual(struct string *s, struct string *t)
{
    SAVE;

    int i;
    if (s==t) {
        RESTORE;
        return 1;
    }

    if (s->length!=t->length) {
        RESTORE;
        return 0;
    }
    for(i=0;i<s->length;i++) 
        if (s->chars[i]!=t->chars[i]) {
            RESTORE;
            return 0;
        }

    RESTORE;
    return 1;

}

void print(struct string *s)
{
    SAVE;
    const unsigned char *p=s->chars;
    for(int i=0;i<s->length;i++,p++) putchar(*p);
    RESTORE;
}

void flush()
{
    SAVE;
    fflush(stdout);
    RESTORE;
}

struct string consts[256];
struct string empty={0,""};

int ord(struct string *s)
{
    SAVE;
    if (s->length==0) { RESTORE; return -1; }
    else { RESTORE; return s->chars[0]; }
}

struct string *chr(int i)
{
    SAVE;
    if (i<0 || i>=256) 
    {printf("chr(%d) out of range\n",i); fflush(stdout); exit(1);}
    RESTORE;
    return consts+i;
}

int size(struct string *s)
{ 
    return s->length;
}

struct string *substring(struct string *s, int first, int n)
{
    SAVE;
    if (first<0 || first+n>s->length)
    {printf("substring([%d],%d,%d) out of range\n",s->length,first,n);
        fflush(stdout);
        exit(1);}
    if (n==1) {
        RESTORE;
        return consts+s->chars[first];
    }
    {struct string *t = (struct string *)malloc(sizeof(int)+n);
        int i;
        t->length=n;
        for(i=0;i<n;i++) t->chars[i]=s->chars[first+i];

        RESTORE;
        return t;
    }

    RESTORE;
}

struct string *concat(struct string *a, struct string *b)
{
    SAVE;

    /*
    printf("STRINGS:\n");
    print(a);
    printf("\n");
    print(b);
    printf("\nEND STRINGS:\n");
    */

    struct string* ret = &empty;
    if (a->length==0) { 
        ret = b;
    }
    else if (b->length==0) { 
        ret = a; 
    } else {
        int i, n=a->length+b->length;
        struct string *t = (struct string *)malloc(sizeof(int)+n);
        t->length=n;
        for (i=0;i<a->length;i++)
            t->chars[i]=a->chars[i];
        for(i=0;i<b->length;i++)
            t->chars[i+a->length]=b->chars[i];
        ret = t;
    }

    RESTORE;
    return ret;
}

int not(int i)
{ 
    return !i;
}


const struct string blah_str = {1, "Z"};
const struct string* blah()
{
    SAVE;
    printf("WOO!\n");
    RESTORE;
    return &blah_str;
}

//const struct string *getchar_()
extern void* getchar(void);
void* getchar(void)
{
    SAVE;

    // No bug, \n is a character too...
    const int i=getc(stdin);
    struct string* ret = &empty;
    if (i != EOF) { ret = &consts[i]; }
    //if (i==EOF) { ret = &empty; }
    //else { ret = &consts[i]; }

    RESTORE;
    return (void*)ret;
}

extern int __prog(void);

int main()
{int i;
    for(i=0;i<256;i++)
    {consts[i].length=1;
        consts[i].chars[0]=i;
    }
    // return tigermain(0 /* static link!? */);
    int res = __prog();
    printf("Result: %u (0x%x)\n", res, res);
    return 0;
}
