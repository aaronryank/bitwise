#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

enum { TYPE_NEG = -1, TYPE_REG = 0, TYPE_FRAME_REG = 1, TYPE_LITERAL = 2, TYPE_LABEL = 3, TYPE_REF = 4, TYPE_ARG = 5 };

int exec_cmd(char *, char *, char *, char *);
int type(char *);
void frame(int);
int _atoi(char *);
void name_function(int,char*);
int call(int,char *,char *,char *);
int32_t _register(char *);
int32_t registers[1000];
int32_t frame_registers[100][1000];
int32_t labels[1000];
int cur_frame;

int line, func;

struct {
  char *name;
  char code[1000][500];
  int lines, fline;
  int labels[100];
  int32_t func_registers[1000];
} functions[200];
int function_depth[201];
int function_count, cur_fdepth;

#define CUR_FUNC function_depth[cur_fdepth]
#define FLINE functions[CUR_FUNC].fline
#define FUNC_REGISTERS functions[CUR_FUNC].func_registers

int main(int argc, char **argv)
{
    FILE *in = (argc >= 2) ? fopen(argv[1],"r") : stdin;

    if (!in) {
        fprintf(stderr,"Invalid source file!");
        return -1;
    }

    int fline = 0, cur_func = 0;
    char s[1000][500] = {0};
    char t[500] = {0};
    while (fgets(t,499,in)) {
        if (strchr(t,'.') == t && strchr(t,':') == t+strlen(t)-2) {   // -2 due to newline
            func = 1;
            fline = 0;
            name_function(cur_func,t);

            continue;
        }

        if (t[strlen(t)-1] == '\n')
           t[strlen(t)-1] = 0;

        if (func) {
            strcpy(functions[cur_func].code[fline],t);
            fline++;
        }
        else {
            strcpy(s[line],t);
            line++;
        }

        if (strstr(t,"RET") == t) {
            func = 0;
            functions[cur_func].lines = fline - 1;
            cur_func++;
        }
    }
    function_count = cur_func;
    int lines = line;
    line = 0;

    while (line < lines) {
        char *t = strdup(s[line]);
        char *cmd = strtok(t," ");
        char *arg1 = strtok(NULL," ");
        char *arg2 = strtok(NULL," ");
        char *arg3 = strtok(NULL," ");

        //printf("%s %s %s %s\n",cmd,arg1,arg2,arg3);

        exec_cmd(cmd,arg1,arg2,arg3);

        free(t);
        line++;
    }
}

int _register(char *index)
{
    if (index[0] == '&')
        return _atoi(index);
    else if (index[0] == '*')
        return frame_registers[cur_frame][_atoi(index)];
    else if (index[0] == '@' && !func)
        return labels[_atoi(index)];
    else if (index[0] == '@' && func)
        return functions[CUR_FUNC].labels[_atoi(index)];
    else if (index[0] == '#')
        return registers[registers[_atoi(index)]];
    else if (index[0] == '$')
        return FUNC_REGISTERS[_atoi(index)];
    else
        return registers[atoi(index)];
}

int exec_cmd(char *cmd, char *arg1, char *arg2, char *arg3)
{
    int result = 0, unrecognized = 0;

    if (!cmd || !strlen(cmd))
        return -1;

    if (!strcmp(cmd,"xor") || !strcmp(cmd,"XOR"))
        result = _register(arg1) ^ _register(arg2);

    else if (!strcmp(cmd,"and") || !strcmp(cmd,"AND"))
        result = _register(arg1) & _register(arg2);

    else if (!strcmp(cmd,"sl") || !strcmp(cmd,"SL"))
        result = _register(arg1) << _register(arg2);

    else if (!strcmp(cmd,"sr") || !strcmp(cmd,"SR"))
        result = _register(arg1) >> _register(arg2);

    else if (!strcmp(cmd,"mov") || !strcmp(cmd,"MOV")) {
        if (_register(arg3)) {
            if (type(arg1) == TYPE_REG)
                registers[atoi(arg1)] = _register(arg2);
            else if (type(arg1) == TYPE_FRAME_REG)
                frame_registers[cur_frame][_atoi(arg1)] = _register(arg2);
            else if (type(arg1) == TYPE_REF)
                registers[registers[_atoi(arg1)]] = _register(arg2);
        }
        return 0;
    }

    else if (!strcmp(cmd,"jmp") || !strcmp(cmd,"JMP")) {
        if (_register(arg2)) {
            int nl = (func ? FLINE : line);
            if (type(arg1) == TYPE_LABEL)
                nl = _register(arg1);
            else {
                nl += _register(arg1);
                if (!_register(arg1))
                    exit(EXIT_SUCCESS);
            }
            //printf(">>> jumping to line %d of %s %d\n",nl,func?"function":"code",func?CUR_FUNC:0);
            if (func)
                FLINE = nl;
            else
                line = nl;
        }
    }

    else if (!strcmp(cmd,"label") || !strcmp(cmd,"LABEL")) {
        if (func)
            functions[CUR_FUNC].labels[_register(arg1)] = FLINE;
        else
            labels[_register(arg1)] = line;
    }

    else if (!strcmp(cmd,"not") || !strcmp(cmd,"NOT")) {
        result = ~_register(arg1);
        arg3 = strdup(arg2);
    }

    else if (!strcmp(cmd,"lsr") || !strcmp(cmd,"LSR"))
        result = (int) ((unsigned int) _register(arg1) >> _register(arg2));    // https://stackoverflow.com/a/5253269/6850771

    else if (!strcmp(cmd,"lor") || !strcmp(cmd,"LOR"))
        result = _register(arg1) || _register(arg2);

    else if (!strcmp(cmd,"land") || !strcmp(cmd,"LAND"))
        result = _register(arg1) && _register(arg2);

    else if (!strcmp(cmd,"in") || !strcmp(cmd,"IN")) {
        if (_register(arg2)) {
            int x = getchar();
            if (type(arg1) == TYPE_FRAME_REG)
                frame_registers[cur_frame][_atoi(arg1)] = x;
            else if (type(arg1) == TYPE_REG)
                registers[atoi(arg1)] = x;
            else if (type(arg1) == TYPE_REF)
                registers[registers[_atoi(arg1)]] = x;
            else if (type(arg1) == TYPE_ARG)
                FUNC_REGISTERS[_atoi(arg1)] = x;
            result = x != -1;
        }   
    }

    else if (!strcmp(cmd,"out") || !strcmp(cmd,"OUT")) {
        if (_register(arg2))
            result = putchar(_register(arg1));
    }

    else if (!strcmp(cmd,"outi") || !strcmp(cmd,"OUTI")) {
        if (_register(arg2))
            result = printf("%d ",_register(arg1));
    }

    else if (!strcmp(cmd,"frame") || !strcmp(cmd,"FRAME"))
        frame(1);

    else if (!strcmp(cmd,"deframe") || !strcmp(cmd,"DEFRAME"))
        frame(-1);

    else if (!strcmp(cmd,"ret") || !strcmp(cmd,"RET"))
        return 0;

    else if (!strcmp(cmd,"__lnot") || !strcmp(cmd,"__LNOT")) {
        result = !_register(arg1);
        arg3 = strdup(arg2);
    }

    else {
        int i, called;
        for (i = called = 0; i < function_count; i++) {
            if (!strcmp(cmd,functions[i].name)) {
                result = call(i,arg1,arg2,arg3);
                called = 1;
            }
        }
        if (!arg2)
            arg2 = strdup(arg1);
        if (!arg3)
            arg3 = strdup(arg2);
        if (!called)
            unrecognized = 1;
    }

    if (!arg1)
        arg1 = strdup("-1");
    else if (!arg2)
        arg2 = strdup("-1");
    else if (!arg3)
        arg3 = strdup("-1");

    if (!strcmp(cmd,"jmp") || !strcmp(cmd,"JMP") || !strcmp(cmd,"label") || !strcmp(cmd,"LABEL") || !strcmp(cmd,"frame") || !strcmp(cmd,"FRAME") || !strcmp(cmd,"deframe") || !strcmp(cmd,"DEFRAME") || unrecognized)
        ;
    else if (type(arg3) == TYPE_REG)
        registers[atoi(arg3)] = result;
    else if (type(arg3) == TYPE_FRAME_REG)
        frame_registers[cur_frame][_atoi(arg3)] = result;
    else if (type(arg3) == TYPE_REF)
        registers[registers[_atoi(arg3)]] = result;
    else if (type(arg3) == TYPE_ARG)
        FUNC_REGISTERS[_atoi(arg3)] = result;

    return 0;
}

void frame(int x)
{
    if (x == 1)
        cur_frame++;
    else if (x == -1) {
        memset(&frame_registers[cur_frame],0,sizeof(int)*1000);
        cur_frame--;
    }
}

// declaration: .foobar:\n
// actual name: foobar
void name_function(int idx, char *decl)
{
    functions[idx].name = strdup(decl);
    char *tmp = strdup(decl);

    strcpy(tmp,decl+1);

    memset(functions[idx].name,0,strlen(decl)*sizeof(char));
    strncpy(functions[idx].name,tmp,strlen(tmp)-2);

    free(tmp);
}

int call(int cmd, char *arg1, char *arg2, char *arg3)
{
    func = 1;
    function_depth[cur_fdepth+1] = cmd;
    functions[function_depth[cur_fdepth+1]].func_registers[0] = cmd;
    functions[function_depth[cur_fdepth+1]].func_registers[1] = _register(arg1);
    functions[function_depth[cur_fdepth+1]].func_registers[2] = _register(arg2);
    frame(1);
    cur_fdepth++;

    char *_cmd, *_arg1, *_arg2, *_arg3, *t;
    for (FLINE = 0; FLINE <= functions[cmd].lines; FLINE++)
    {
        t = strdup(functions[cmd].code[FLINE]);
        _cmd = strtok(t," ");
        _arg1 = strtok(NULL," ");
        _arg2 = strtok(NULL," ");
        _arg3 = strtok(NULL," ");

        //printf("%d: executing line %d of function %d: %s %s %s %s\n",cur_fdepth,FLINE,cmd,_cmd,_arg1,_arg2,_arg3);
        exec_cmd(_cmd,_arg1,_arg2,_arg3);

        if (!strcmp(_cmd,"ret") || !strcmp(_cmd,"RET"))
            break;

        free(t);
    }

    int retval = _register(_arg1);
    //printf("%d: returning %d\n",cur_fdepth,retval);
    memset(&FUNC_REGISTERS,0,sizeof(int)*1000);
    free(t);
    frame(-1);
    CUR_FUNC = 0;
    cur_fdepth--;
    if (!cur_fdepth)
        func = 0;
    return retval;
}

int type(char *val)
{
    if (*val == '-')
        return -1;
    else if (isdigit(*val))
        return 0;
    else if (*val == '*')
        return 1;
    else if (*val == '&')
        return 2;
    else if (*val == '@')
        return 3;
    else if (*val == '#')
        return 4;
    else if (*val == '$')
        return 5;
    return -2;
}

int _atoi(char *str)
{
    char *num = strdup(str);
    strcpy(num,&str[1]);
    return atoi(num);
}
