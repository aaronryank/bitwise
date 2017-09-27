#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

int exec_cmd(char *, char *, char *, char *);
int type(char *);
void frame(int);
int _atoi(char *);
int32_t _register(char *);
int32_t registers[1000];
int32_t frame_registers[100][1000];
int32_t labels[1000];
int cur_frame;

int line;

int main(int argc, char **argv)
{
    FILE *in = (argc >= 2) ? fopen(argv[1],"r") : stdin;

    if (!in) {
        fprintf(stderr,"Invalid source file!");
        return -1;
    }

    char s[1000][500] = {0};
    while (fgets(s[line],499,in)) {
        if (s[line][strlen(s[line])-1] == '\n')
            s[line][strlen(s[line])-1] = 0;
        line++;
    }
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
    else if (index[0] == '@')
        return labels[_atoi(index)];
    else if (index[0] == '#')
        return registers[registers[_atoi(index)]];
    else
        return registers[atoi(index)];
}

int exec_cmd(char *cmd, char *arg1, char *arg2, char *arg3)
{
    int result = 0;

    if (!strcmp(cmd,"xor") || !strcmp(cmd,"XOR"))
        result = _register(arg1) ^ _register(arg2);

    if (!strcmp(cmd,"and") || !strcmp(cmd,"AND"))
        result = _register(arg1) & _register(arg2);

    else if (!strcmp(cmd,"sl") || !strcmp(cmd,"SL"))
        result = _register(arg1) << _register(arg2);

    else if (!strcmp(cmd,"sr") || !strcmp(cmd,"SR"))
        result = _register(arg1) >> _register(arg2);

    else if (!strcmp(cmd,"mov") || !strcmp(cmd,"MOV")) {
        if (_register(arg3)) {
            if (type(arg1) == 0)
                registers[atoi(arg1)] = _register(arg2);
            else if (type(arg1) == 1)
                frame_registers[cur_frame][_atoi(arg1)] = _register(arg2);
            else if (type(arg1) == 4)
                registers[registers[_atoi(arg1)]] = _register(arg2);
        }
        return 0;
    }

    else if (!strcmp(cmd,"jmp") || !strcmp(cmd,"JMP")) {
        if (_register(arg2)) {
            if (type(arg1) == 3)
                line = _register(arg1);
            else {
                line += _register(arg1);
                if (!_register(arg1))
                    exit(EXIT_SUCCESS);
            }
        }
    }

    else if (!strcmp(cmd,"label") || !strcmp(cmd,"LABEL"))
        labels[_register(arg1)] = line;

    else if (!strcmp(cmd,"not") || !strcmp(cmd,"NOT")) {
        result = ~_register(arg1);
        arg3 = strdup(arg2);
    }

    else if (!strcmp(cmd,"lnot") || !strcmp(cmd,"LNOT")) {
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
            if (type(arg1) == 1)
                frame_registers[cur_frame][_atoi(arg1)] = x;
            else if (type(arg1) == 0)
                registers[atoi(arg1)] = x;
            else if (type(arg1) == 4)
                registers[registers[_atoi(arg1)]] = x;
            result = x != -1;
        }   
    }

    else if (!strcmp(cmd,"out") || !strcmp(cmd,"OUT")) {
        if (_register(arg2))
            result = putchar(_register(arg1));
    }

    else if (!strcmp(cmd,"frame") || !strcmp(cmd,"FRAME"))
        frame(1);

    else if (!strcmp(cmd,"deframe") || !strcmp(cmd,"DEFRAME"))
        frame(-1);

    if (!arg1)
        arg1 = strdup("-1");
    else if (!arg2)
        arg2 = strdup("-1");
    else if (!arg3)
        arg3 = strdup("-1");

    if (!strcmp(cmd,"jmp") || !strcmp(cmd,"JMP") || !strcmp(cmd,"label") || !strcmp(cmd,"LABEL") || !strcmp(cmd,"frame") || !strcmp(cmd,"FRAME") || !strcmp(cmd,"deframe") || !strcmp(cmd,"DEFRAME"))
        ;
    else if (type(arg3) == 0)
        registers[atoi(arg3)] = result;
    else if (type(arg3) == 1)
        frame_registers[cur_frame][_atoi(arg3)] = result;
    else if (type(arg3) == 4)
        registers[registers[_atoi(arg3)]] = result;

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
    return -2;
}

int _atoi(char *str)
{
    char *num = strdup(str);
    strcpy(num,&str[1]);
    return atoi(num);
}
