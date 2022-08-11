#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include "9cc.h"

extern FILE *tout;
extern FILE *tout2;
extern char* user_input;
extern char* filename;
extern Token* token;
extern Node *code[];
extern LVar *locals,*globals,*strings;

char* read_file(char *path){
        FILE*fp=fopen(path,"r");
        if(!fp)
                error("cannot open %s:",path,strerror(errno));

        if(fseek(fp,0,SEEK_END)==-1)
                error("%s:fseek:%s",path,strerror(errno));
        size_t size=ftell(fp);
        if(fseek(fp,0,SEEK_SET)==-1)
                error("%s:fseek:%s",path,strerror(errno));
        char *buf=calloc(sizeof(char),size+2);
        fread(buf,size,1,fp);

        //make sure that buffer end in \n\0
        if(size==0||buf[size-1]!='\n')
                buf[size++]='\n';
        buf[size]='\0';
        fclose(fp);
        return buf;
}
int main(int argc,char **argv){
       tout2=stdout;//debug
       //tout=stderr;
    if(argc!=2){
        fprintf(stderr,"wrong number of argument\n.");
        return 1;
    }
    tout=fopen("tmp.xml","w");
    locals=calloc(1,sizeof(LVar));
    filename=argv[1];
    //fprintf(tout,"# %s\n",filename);
    user_input=read_file(filename);

    token=tokenize(user_input);
    program();

    //header
    printf(".intel_syntax noprefix\n");

    for(LVar *var=strings;var;var=var->next){
        //printf("  .text \n");
      printf(".LC%d:\n",var->offset);
      printf("  .string \"%s\"\n",var->name);
      //printf("  .text \n");
    }

    for(LVar *var=globals;var;var=var->next){//gvar
           //https://github.com/rui314/chibicc/commit/a4d3223a7215712b86076fad8aaf179d8f768b14
           printf(".data\n");
           printf(".global %s\n",var->name);
           printf("%s:\n",var->name);
           if(var->type->kind==TY_INT){
                   printf("  .zero 4\n");
           }else if(var->type->kind==TY_PTR){
                   printf("  .zero 8\n");
           }else{
                   printf("  .zero %d\n",var->type->array_size*4);
           }
    }


    printf(".global main\n");
    for(int i=0;code[i];i++){
            gen(code[i]);

            //pop each result in order not to over flow
            printf("  pop rax\n");
    }

    return 0;
}
