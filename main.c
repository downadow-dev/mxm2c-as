
/* mxm2c-as — это Makexm2c-ассемблер, который должен превзойти
   по скорости и эффективности ассемблер из makexm2c-tools.
   
   --downadow
 */


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

struct as_entry {
    char name[100];
    char value[100];
    struct as_entry *next;
};

struct as_entry *labels;
struct as_entry *refs;

int preprocess(char *);

int main(int argc, char **argv) {
    if(argc < 3) {
        fprintf(stderr, "usage: %s IN_FILE OUT_FILE\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    int *app = malloc(10000000 * sizeof(int));
    memset(app, '\0', 10000000 * sizeof(int));
    
    labels = malloc(sizeof(struct as_entry));
    labels->next = NULL;
    refs = malloc(sizeof(struct as_entry));
    refs->next = NULL;
    
    FILE *f;
    if(!(f = fopen(argv[1], "r"))) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    
    char buf[4096];
    // работа для создания меток и ссылок
    for(int i = 0; fgets(buf, sizeof(buf), f);) {
        char *s = buf;
        
        preprocess(s);
        
        if(*s == '\0')
            continue;
        
        char *instr = s;
        
        s = strchr(s, ' ');
        if(s) *s++ = '\0';
        
        /* заполнения для соответствия двоичному коду */
        if(strcmp(instr, "nop") == 0  || strcmp(instr, "off") == 0  ||
           strcmp(instr, "updd") == 0 || strcmp(instr, ".byte") == 0  ||
           strcmp(instr, "vrst") == 0 || strcmp(instr, "trst") == 0) i++;
        else if(strcmp(instr, "add") == 0 ||
                strcmp(instr, "lshift") == 0 ||
                strcmp(instr, "rshift") == 0 ||
                strcmp(instr, "xor") == 0 ||
                strcmp(instr, "or") == 0 ||
                strcmp(instr, "and") == 0 ||
                strcmp(instr, "sub") == 0 ||
                strcmp(instr, "mod") == 0 ||
                strcmp(instr, "mul") == 0 ||
                strcmp(instr, "div") == 0 ||
                strcmp(instr, "if") == 0) i += 4;
        else if(strcmp(instr, "mov") == 0 ||
                strcmp(instr, "mov2") == 0 ||
                strcmp(instr, "call") == 0 ||
                strcmp(instr, "isv") == 0 ||
                strcmp(instr, "vsv") == 0 ||
                strcmp(instr, "ild") == 0 ||
                strcmp(instr, "vld") == 0 ||
                strcmp(instr, "ld") == 0) i += 3;
        else if(strcmp(instr, "inc") == 0 ||
                strcmp(instr, "dec") == 0 ||
                strcmp(instr, "tnp") == 0 ||
                strcmp(instr, "time") == 0 ||
                strcmp(instr, "jmp") == 0) i += 2;
        else if(strcmp(instr, ".skip") == 0) i += strtol(s, NULL, 0);
        else if(strcmp(instr, ".orig") == 0) i = strtol(s, NULL, 0);
        // создание метки
        else if(instr[strlen(instr) - 1] == ':') {
            instr[strlen(instr) - 1] = '\0';
            struct as_entry *ent = labels;
            for(;;) {
                if(ent->next) {
                    ent = ent->next;
                    continue;
                }
                ent->next = malloc(sizeof(struct as_entry));
                strcpy(ent->next->name, instr);
                sprintf(ent->next->value, "%07d", i);
                ent->next->next = NULL;
                
                printf("%s:%d\n", instr, i);
                
                break;
            }
        }
        // создание именованной константы
        else if(strcmp(instr, ".def") == 0) {
            char *name = s + 1;
            *strchr(name, '%') = '\0';
            char *value = name + strlen(name) + 3;
            *strchr(value, '"') = '\0';
            struct as_entry *ent = refs;
            for(;;) {
                if(ent->next) {
                    ent = ent->next;
                    continue;
                }
                ent->next = malloc(sizeof(struct as_entry));
                strcpy(ent->next->name, name);
                strcpy(ent->next->value, value);
                ent->next->next = NULL;
                break;
            }
        }
    }
    
    fclose(f);
    
    if(!(f = fopen(argv[1], "r"))) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    
    // перевод программы в машинный код
    int i;
    for(i = 0; fgets(buf, sizeof(buf), f);) {
        char *s = buf;
        
        int status = preprocess(s);
        switch(status) {
        case -1:
            fprintf(stderr, "preprocess() warning (unknown label): %s\n", s);
            break;
        case -2:
            fprintf(stderr, "preprocess() warning (unknown def): %s\n", s);
            break;
        }
        
        if(*s == '\0')
            continue;
        
        char *instr = s;
        
        s = strchr(s, ' ');
        if(s) *s++ = '\0';
        
        if(strcmp(instr, "nop") == 0) app[i++] = -1;
        else if(strcmp(instr, "add") == 0) {
            int r1 = atoi(strsep(&s, " "));
            int r2 = atoi(strsep(&s, " "));
            int rdst = atoi(strsep(&s, " "));
            
            app[i++] = r2;
            app[i++] = r1;
            app[i++] = rdst;
            app[i++] = -2;
        }
        else if(strcmp(instr, "lshift") == 0) {
            int r1 = atoi(strsep(&s, " "));
            int r2 = atoi(strsep(&s, " "));
            int rdst = atoi(strsep(&s, " "));
            
            app[i++] = r2;
            app[i++] = r1;
            app[i++] = rdst;
            app[i++] = -38;
        }
        else if(strcmp(instr, "rshift") == 0) {
            int r1 = atoi(strsep(&s, " "));
            int r2 = atoi(strsep(&s, " "));
            int rdst = atoi(strsep(&s, " "));
            
            app[i++] = r2;
            app[i++] = r1;
            app[i++] = rdst;
            app[i++] = -39;
        }
        else if(strcmp(instr, "xor") == 0) {
            int r1 = atoi(strsep(&s, " "));
            int r2 = atoi(strsep(&s, " "));
            int rdst = atoi(strsep(&s, " "));
            
            app[i++] = r2;
            app[i++] = r1;
            app[i++] = rdst;
            app[i++] = -40;
        }
        else if(strcmp(instr, "or") == 0) {
            int r1 = atoi(strsep(&s, " "));
            int r2 = atoi(strsep(&s, " "));
            int rdst = atoi(strsep(&s, " "));
            
            app[i++] = r2;
            app[i++] = r1;
            app[i++] = rdst;
            app[i++] = -41;
        }
        else if(strcmp(instr, "and") == 0) {
            int r1 = atoi(strsep(&s, " "));
            int r2 = atoi(strsep(&s, " "));
            int rdst = atoi(strsep(&s, " "));
            
            app[i++] = r2;
            app[i++] = r1;
            app[i++] = rdst;
            app[i++] = -42;
        }
        else if(strcmp(instr, "sub") == 0) {
            int r1 = atoi(strsep(&s, " "));
            int r2 = atoi(strsep(&s, " "));
            int rdst = atoi(strsep(&s, " "));
            
            app[i++] = r2;
            app[i++] = r1;
            app[i++] = rdst;
            app[i++] = -3;
        }
        else if(strcmp(instr, "mov") == 0 || strcmp(instr, "mov2") == 0) {
            int rdst = atoi(strsep(&s, " "));
            int src = atoi(strsep(&s, " "));
            
            app[i++] = src;
            app[i++] = rdst;
            app[i++] = -4;
        }
        else if(strcmp(instr, "ild") == 0) {
            char *src = strsep(&s, " ");
            int rdst = atoi(strsep(&s, " "));
            
            app[i++] = atoi(src);
            app[i++] = rdst;
            
            app[i++] = (strlen(src) > 3) ? -5 : -47;
        }
        else if(strcmp(instr, "vld") == 0) {
            char *src = strsep(&s, " ");
            int rdst = atoi(strsep(&s, " "));
            
            app[i++] = atoi(src);
            app[i++] = rdst;
            
            app[i++] = (strlen(src) > 3) ? -6 : -48;
        }
        else if(strcmp(instr, "isv") == 0) {
            int rsrc = atoi(strsep(&s, " "));
            char *dst = strsep(&s, " ");
            
            app[i++] = rsrc;
            app[i++] = atoi(dst);
            
            app[i++] = (strlen(dst) > 3) ? -8 : -45;
        }
        else if(strcmp(instr, "vsv") == 0) {
            int rsrc = atoi(strsep(&s, " "));
            char *dst = strsep(&s, " ");
            
            app[i++] = rsrc;
            app[i++] = atoi(dst);
            
            app[i++] = (strlen(dst) > 3) ? -9 : -46;
        }
        else if(strcmp(instr, "ld") == 0) {
            app[i++] = atoi(strsep(&s, " "));
            app[i++] = atoi(strsep(&s, " "));
            app[i++] = -7;
        }
        else if(strcmp(instr, "if") == 0) {
            int op1 = atoi(strsep(&s, " "));
            char type = *strsep(&s, " ");
            int op2 = atoi(strsep(&s, " "));
            int rdst = atoi(strsep(&s, " "));
            
            app[i++] = rdst;
            app[i++] = op2;
            app[i++] = op1;
            
            switch(type) {
            case '=': app[i++] = -11; break;
            case '!': app[i++] = -12; break;
            case '>': app[i++] = -13; break;
            case '<': app[i++] = -14; break;
            }
        }
        else if(strcmp(instr, "updd") == 0) app[i++] = -15;
        else if(strcmp(instr, "off") == 0)  app[i++] = -16;
        else if(strcmp(instr, "vrst") == 0) app[i++] = -17;
        else if(strcmp(instr, "trst") == 0) app[i++] = -44;
        else if(strcmp(instr, "jmp") == 0) {
            app[i++] = atoi(s);
            app[i++] = -18;
        }
        else if(strcmp(instr, "call") == 0) {
            int reg = atoi(strsep(&s, " "));
            int addr = atoi(strsep(&s, " "));
            
            app[i++] = addr;
            app[i++] = reg;
            app[i++] = -10;
        }
        else if(strcmp(instr, "mul") == 0) {
            int r1 = atoi(strsep(&s, " "));
            int r2 = atoi(strsep(&s, " "));
            int rdst = atoi(strsep(&s, " "));
            
            app[i++] = r2;
            app[i++] = r1;
            app[i++] = rdst;
            app[i++] = -21;
        }
        else if(strcmp(instr, "div") == 0) {
            int r1 = atoi(strsep(&s, " "));
            int r2 = atoi(strsep(&s, " "));
            int rdst = atoi(strsep(&s, " "));
            
            app[i++] = r2;
            app[i++] = r1;
            app[i++] = rdst;
            app[i++] = -22;
        }
        else if(strcmp(instr, "inc") == 0) {
            app[i++] = atoi(s);
            app[i++] = -26;
        }
        else if(strcmp(instr, "dec") == 0) {
            app[i++] = atoi(s);
            app[i++] = -27;
        }
        else if(strcmp(instr, "tnp") == 0) {
            app[i++] = atoi(s);
            app[i++] = -28;
        }
        else if(strcmp(instr, "mod") == 0) {
            int r1 = atoi(strsep(&s, " "));
            int r2 = atoi(strsep(&s, " "));
            int rdst = atoi(strsep(&s, " "));
            
            app[i++] = r2;
            app[i++] = r1;
            app[i++] = rdst;
            app[i++] = -29;
        }
        else if(strcmp(instr, "time") == 0) {
            app[i++] = atoi(s);
            app[i++] = -43;
        }
        
        
        else if(strcmp(instr, ".ascii") == 0) {
            char *p = strchr(s, ' ');
            *p++ = '\0';
            *p++ = '\0';
            *strchr(p, '"') = '\0';
            int j = atoi(s);
            while(*p) app[j++] = *p++;
            app[j] = '\0';
        }
        else if(strcmp(instr, ".skip") == 0) i += atoi(s);
        else if(strcmp(instr, ".orig") == 0) {
            int new = atoi(s);
            if(i > new) fprintf(stderr, ".orig: warning: possible overflow\n");
            i = new;
        }
        else if(strcmp(instr, ".byte") == 0) app[i++] = atoi(s);
    }
    fclose(f);
    
    if(!(f = fopen(argv[2], "w"))) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    
    for(int j = 0; j < i; j++)
        fprintf(f, "%d\n", app[j]);
    
    fclose(f);
}

// преобразовать в s_buf имена меток и именованных констант в их значения и др.
int preprocess(char *s_buf) {
    while(isspace(*s_buf))
        memmove(s_buf, s_buf + 1, strlen(s_buf + 1) + 1);
    
    for(int i = strlen(s_buf) - 1; i > 0; i--) {
        if(isspace(s_buf[i]))
            s_buf[i] = '\0';
        else
            break;
    }
    
    if(*s_buf && s_buf[strlen(s_buf) - 1] == ':')
        return 0;
    
    int lrblock = 0;
    
    for(char *p = s_buf; *p; p++) {
        if((*p == '<' || *p == '%') && !isspace(p[1]) && !lrblock)
            lrblock = 1;
        else if((*p == '>' || *p == '%') && !isspace(p[-1]) && lrblock)
            lrblock = 0;
        
        if(!lrblock) {
            if(isupper(*p))
                *p = tolower(*p);
            else if(*p == ',')
                *p = ' ';
            else if(p[0] == '\'' && p[1] && p[2] == '\'') {
                char tmp[4];
                sprintf(tmp, "%d", (int)p[1]);
                memmove(p + strlen(tmp), p + 3, strlen(p + 3) + 1);
                memcpy(p, tmp, strlen(tmp));
            }
            else if(*p == '"')
                break;
        }
    }
    
    lrblock = 0;
    
    for(char *p = s_buf; *p;) {
        if((*p == '<' || *p == '%') && !isspace(p[1]) && !lrblock)
            lrblock = 1;
        else if((*p == '>' || *p == '%') && !isspace(p[-1]) && lrblock)
            lrblock = 0;
        
        if(!lrblock) {
            if(isspace(p[0]) && isspace(p[1]))
                memmove(p, p + 1, strlen(p + 1) + 1);
            else if(p[0] == 'u' && p[1] == 'r')
                memmove(p, p + 2, strlen(p + 2) + 1);
            else if(*p == '"')
                break;
            else p++;
        } else p++;
    }
    
    for(char *p = s_buf; *p; p++) {
        if(*p == '<' && !isspace(p[1])) {
            char *endp;
            for(endp = p + 1; *endp; endp++) {
                if(*endp == '>') {
                    char buf[100];
                    memset(buf, '\0', sizeof(buf));
                    memcpy(buf, p + 1, (endp - 1) - p);
                    
                    struct as_entry *ent = labels;
                    while(ent = ent->next) {
                        if(strcmp(ent->name, buf) == 0) {
                            memmove(p + strlen(ent->value), endp + 1, strlen(endp) + 1);
                            memcpy(p, ent->value, strlen(ent->value));
                            goto labelok;
                        }
                    }
                    
                    return -1;
                    
                    labelok: break;
                }
            }
        } else if(*p == '%' && !(s_buf[0] == '.' && s_buf[1] == 'd' && s_buf[2] == 'e' && s_buf[3] == 'f')) {
            char *endp;
            for(endp = p + 1; *endp; endp++) {
                if(*endp == '%') {
                    char buf[100];
                    memset(buf, '\0', sizeof(buf));
                    memcpy(buf, p + 1, (endp - 1) - p);
                    
                    struct as_entry *ent = refs;
                    while(ent = ent->next) {
                        if(strcmp(ent->name, buf) == 0) {
                            memmove(p + strlen(ent->value), endp + 1, strlen(endp) + 1);
                            memcpy(p, ent->value, strlen(ent->value));
                            preprocess(s_buf);
                            goto defok;
                        }
                    }
                    
                    return -2;
                    
                    defok: break;
                }
            }
        }
    }
    
    return 0;
}
