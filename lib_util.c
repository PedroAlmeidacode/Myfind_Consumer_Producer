//
// Created by pedro on 21/05/20.
//
#include "library.h"


int isSocket(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISSOCK(statbuf.st_mode);
}

int isCharater(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISCHR(statbuf.st_mode);
}

int isBlock(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISBLK(statbuf.st_mode);
}

int isSymbolicLink(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISLNK(statbuf.st_mode);
}

int isFile(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISREG(statbuf.st_mode);
}

int isDirectory(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISDIR(statbuf.st_mode);
}

int isFifo(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISFIFO(statbuf.st_mode);

}

char *stringLwr(char *s) {
    int len, i;
    len = strlen(s);

    for (i = 0; i < len; i++) {
        s[i] = tolower(s[i]);

    }
    return s;
}


/**
 * Testa se o ficheiro e maior ou menor (especificado em above_size)
 * size contem o tamanho em Megas
 * Se size estiver a 0 significa que nao foi especificado nos argumentos
 * @param size tamanho
 * @param above_size true para acima de size , false para abaixo de size
 * @param path ca,imho para o ficheiro a testar
 * @return true se estiver nos tamanbos estipulados
 */
int aboveorUnderSize(char *value, const char *path, char *d_name) {

    bool above_size = false;
    if (value[0] == '+') above_size = true;
    char *sz = malloc(10);
    strcpy(sz, value);
    memmove(&sz[0], &sz[1], strlen(sz)); //remove a primeira posicao
    sz[strlen(sz) - 1] = '\0';
    int size = atoi(sz);
    if (size == 0) return 1; // se nao tiver sido explicito nada nos argumentos deste parametro
    //size expressed in Megabytes
    long bytes_size = size * 1048576;
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    if (above_size == true) {
        return statbuf.st_size > bytes_size;
    } else return statbuf.st_size < bytes_size;
}

/**
 * Testa se o ficheiro foi modificado nos ultmos nnmin minutos
 * Se mmin estiver a 0 significa que nao foi especificado nos argumentos
 * e sendo assim retorna true
 * @param nnmin numero de minutos
 * @param path caminho para o file a testar
 * @return true se o ficheiro tiver sido modificado dentro do tempo modifcado
 */
int mmin(char *value, const char *path, char *d_name) {
    int nnmin = atoi(value);
    if (nnmin == 0) return 1;  // se nao tiver sido explicito nada nos argumentos deste parametro
    struct stat file_stat;

    int err = stat(path, &file_stat);
    if (err != 0) {
        perror(" [file_is_modified] stat");
        exit(errno);
    }
    struct tm time_of_mod;

    stat(path, &file_stat);
    time_of_mod = *gmtime(&(file_stat.st_mtime)); // obtem o tempo da ultima modificacao do ficheiro

    //time_t t = time(NULL);
    //struct tm *tm = localtime(&t); // get current time
    time_t now;
    time(&now);

    // obtem o tempo pedido, o ficheiro nao deve ter sido criado antes deste tempo para retornar verdadeiro

    double difference_in_seconds = difftime(now, mktime(&time_of_mod));
    double seconds_of_argv = nnmin * 60; // each minute has 60 seconds
    if (difference_in_seconds < seconds_of_argv) return 1;
    return 0;
}

/**
 * Se executable tiver null significa que nao foi especificado nos args
 * e sendo assim retorna true
 * @param executable parametro especificado em args
 * @param path path para o ficheiro a testar
 * @return true se o ficheiro for executavel
 */
int executable(char *value, const char *path, char *d_name) {
    struct stat statbuf;
    return stat(path, &statbuf) == 0 && statbuf.st_mode & S_IXUSR ? 1 : 0;
}

/**
 * Se empty tiver null significa que nao foi especificado nos args
 * e sendo assim retorna true
 * Testa se o ficheiro passado em path esta vazio ou nao
 * Se estiver vazio retorna true
 *
 * @param empty paramtro especificado em args
 * @param path path para o ficheiro a testar
 * @return true se o ficheiro tiver vazio
 */
int empty(char *value, const char *path, char *d_name) {
    struct stat statbuf;
    return stat(path, &statbuf) != 0 ? 0 : statbuf.st_size == 0;
    // se o tamanho for 0 retorna 1
}

/**
 * Se type tiver x que significa que nao foi especificado nos args
 * e sendo assim retorna true
 * De acordo com o caracter em type testa se o ficheiro incluido em path
 * verifica se
 *
 * @param type tipo escolhido em args
 * @param path path para o ficheiro
 * @return veradadeiro se for do tipo escolhido
 */
int type(char *value, const char *path, char *d_name) {
    char type = value[0];
    switch (type) {
        case 'c':
            if (isCharater(path)) {
                return 1;
            } else return 0;
        case 'b':
            if (isBlock(path)) {
                return 1;
            } else return 0;
        case 'd':
            if (isDirectory(path)) {
                return 1;
            } else return 0;
        case 'p':
            if (isFifo(path)) {
                return 1;
            } else return 0;
        case 'f':
            if (isFile(path)) {
                return 1;
            } else return 0;
        case 'l':
            if (isSymbolicLink(path)) {
                return 1;
            } else return 0;
        case 's':
            if (isSocket(path)) {
                return 1;
            } else return 0;
        default:
            return 0;
    }
}


int iName(char *value, const char *path, char *d_name) {
    d_name = stringLwr(d_name); // passa o nome do ficheiro encontrado para minuscula
    if (strcmp(value, d_name) == 0) {
        return 1;
    } else return 0;

}

int Name(char *value, const char *path, char *d_name) {
    if (strcmp(value, d_name) == 0) {
        return 1;
    } else return 0;

}

int p_name(char *value, const char *path, char *d_name) {
    // testar se prefixo == a primeira parte de d_name
    for (int i = 0; i < strlen(value); i++) {
        if (d_name[i] != value[i]) {
            return 0; // se for diferente retorna logo falso
        }
    }
    return 1; // se sair fora do for quer dizer que pref e igual as primeiras strlen(pref) posicoes de pref
}

int p_iname(char *value, const char *path, char *d_name) {
    d_name = stringLwr(d_name); // passa o nome do ficheiro encontrado para minuscula
    // testar se prefixo == a primeira parte de d_name
    for (int i = 0; i < strlen(value); i++) {
        if (d_name[i] != value[i]) {
            return 0; // se for diferente retorna logo falso
        }
    }
    return 1; // se sair fora do for quer dizer que pref e igual as primeiras strlen(pref) posicoes de pref
}

int s_name(char *value, const char *path, char *d_name) {
    // testar se o sufixo  == a ultima parte de d_name
    int last_pos_suf = strlen(value) - 1;
    int last_pos_name = strlen(d_name) - 1;
    while (last_pos_suf >= 0) {
        if (d_name[last_pos_name] != value[last_pos_suf]) {
            return 0; // se for diferente retorna logo falso
        }
        last_pos_name--;
        last_pos_suf--;
    }
    return 1;
}

int s_iname(char *value, const char *path, char *d_name) {
    d_name = stringLwr(d_name); // passa o nome do ficheiro encontrado para minuscula
    // testar se o sufixo  == a ultima parte de d_name
    int last_pos_suf = strlen(value) - 1;
    int last_pos_name = strlen(d_name) - 1;
    while (last_pos_suf >= 0) {
        if (d_name[last_pos_name] != value[last_pos_suf]) {
            return 0; // se for diferente retorna logo falso
        }
        last_pos_name--;
        last_pos_suf--;
    }
    return 1;
}

void parse_args(int argc, char *argv[], DATA *data) {

    char path_wd[200];
    getcwd(path_wd, sizeof(path_wd));

    if (argc == 1) {
        printf("Nao foram especificados argumentos\n");
    }
    if (argc >= 2) {
        char tmp = argv[1][0]; // para checkar se a primeria posicao e um ponto
        if (tmp == '.') {
            if (strlen(argv[1]) == 1) {
                data->path = malloc(strlen(path_wd) + 1);
                sprintf(data->path, "%s", path_wd);
            } else {
                char *path_arg = argv[1];
                memmove(&path_arg[0], &path_arg[1], strlen(path_arg)); //remove a primeira posicao
                data->path = malloc(strlen(path_wd) + strlen(path_arg));
                sprintf(data->path, "%s%s", path_wd, path_arg);
            }
        } else { // se for uma directoria em especifico
            data->path = malloc(strlen(argv[1]));
            data->path = argv[1];
        }
        int i = 2;
        while (i < argc) {
            if (strcmp(argv[i], "-name") == 0) {
                char *sufix_name;
                char *prefix_name;
                char *name;
                if (argv[i + 1][0] == '*') {

                    data->args[data->n_args].opt = (PARAM) s_name;
                    sufix_name = malloc(sizeof(argv[i + 1]) - 1);
                    int iterator = 0;
                    for (int j = 1; j < strlen(argv[i + 1]); j++) {
                        sufix_name[iterator] = argv[i + 1][j];
                        iterator++;
                    }
                    data->args[data->n_args].value = sufix_name;
                    data->n_args++;
                }
                    // indica apenas o prefixo do nome
                else if (argv[i + 1][strlen(argv[i + 1]) - 1] == '*') {
                    data->args[data->n_args].opt = (PARAM) p_name;
                    prefix_name = malloc(sizeof(argv[i + 1]) - 1);
                    int iterator = 0;
                    for (int j = 0; j < strlen(argv[i + 1]) - 1; j++) {
                        prefix_name[iterator] = argv[i + 1][j];
                        iterator++;
                    }
                    data->args[data->n_args].value = prefix_name;
                    data->n_args++;

                } else {
                    data->args[data->n_args].opt = (PARAM) Name;
                    name = malloc(sizeof(argv[i + 1]));
                    strcpy(name, argv[i + 1]);
                    data->args[data->n_args].value = name;
                    data->n_args++;
                }
                i += 2;
            } else if (strcmp(argv[i], "-iname") == 0) {
                char *sufix_iname;
                char *prefix_iname;
                char *iname;
                // indica apenas o sufixo do nome
                if (argv[i + 1][0] == '*') {
                    data->args[data->n_args].opt = (PARAM) s_iname;
                    sufix_iname = malloc(sizeof(argv[i + 1]) - 1);
                    int iterator = 0;
                    for (int j = 1; j < strlen(argv[i + 1]); j++) {
                        sufix_iname[iterator] = argv[i + 1][j];
                        iterator++;
                    }
                    sufix_iname = stringLwr(sufix_iname);
                    data->args[data->n_args].value = sufix_iname;
                    data->n_args++;
                }
                    // indica apenas o prefixo do nome
                else if (argv[i + 1][strlen(argv[i + 1]) - 1] == '*') {
                    data->args[data->n_args].opt = (PARAM) p_iname;
                    prefix_iname = malloc(sizeof(argv[i + 1]) - 1);
                    int iterator = 0;
                    for (int j = 0; j < strlen(argv[i + 1]) - 1; j++) {
                        prefix_iname[iterator] = argv[i + 1][j];
                        iterator++;
                    }
                    prefix_iname = stringLwr(prefix_iname);
                    data->args[data->n_args].value = prefix_iname;
                    data->n_args++;
                } else {
                    data->args[data->n_args].opt = (PARAM) iName;
                    iname = malloc(sizeof(argv[i + 1]));
                    strcpy(iname, argv[i + 1]);
                    iname = stringLwr(iname);
                    data->args[data->n_args].value = iname;
                    data->n_args++;
                }
                i += 2;

            } else if (strcmp(argv[i], "-type") == 0) {
                data->args[data->n_args].opt = (PARAM) type;
                char carecter = argv[i + 1][0];
                // se for differente de todas nao esta dentro das escolhas para o tipo
                if (carecter != 'c' && carecter != 'b' && carecter != 'd' && carecter != 'p' && carecter != 'f' &&
                    carecter != 'l' && carecter != 's') {
                    printf("type choosed is not acepted\n");
                    exit(EXIT_FAILURE);
                }
                char *car = malloc(1);
                strcpy(car, &carecter);
                data->args[data->n_args].value = car;
                data->n_args++;
                i += 2;
            } else if (strcmp(argv[i], "-empty") == 0) {
                data->args[data->n_args].opt = (PARAM) empty;
                data->args[data->n_args].value = "true";
                data->n_args++;
                i++;
            } else if (strcmp(argv[i], "-executable") == 0) {
                data->args[data->n_args].opt = (PARAM) executable;
                data->args[data->n_args].value = "true";
                data->n_args++;
                i++;
            } else if (strcmp(argv[i], "-mmin") == 0) {
                char *time = argv[i + 1];
                memmove(&time[0], &time[1], strlen(time)); //remove a primeira posicao
                data->args[data->n_args].opt = (PARAM) mmin;
                data->args[data->n_args].value = time;
                data->n_args++;
                i += 2;
            } else if (strcmp(argv[i], "-size") == 0) {
                char *sz = argv[i + 1];
                data->args[data->n_args].opt = (PARAM) aboveorUnderSize;
                data->args[data->n_args].value = sz;
                data->n_args++;
                i += 2;
            } else {
                printf("Argumento %s nao valido\n", argv[i]);
                break;
            }
        }
    }
}
