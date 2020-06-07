#include "library.h"


int prodptr = 0, consptr = 0;

pthread_mutex_t trinco_p = PTHREAD_MUTEX_INITIALIZER; // trinco produtor
pthread_mutex_t trinco_c = PTHREAD_MUTEX_INITIALIZER; // trinco consumidor
pthread_mutex_t trinco = PTHREAD_MUTEX_INITIALIZER; // trinco de matches

DATA data[N]; //struct de data com N poisicoes onde pode ser produzido e consumido
THREADS *head = NULL; // vai guardar a primeria posicao da linked list threads
int n_threads = 0; // variavel global que guarda o numero de struct thread criadas (nao o numero de threads lancadas)
sem_t semPodeCons, semPodeProd;

// main tenta perceber se nenhum deles esta a consumir e estao todos a espera de recenber , se o produtor ja tivwer termiando faz pthread_detach de todos os threads consumidores


THREADS *create_thread_and_match(pthread_t self, char *new_path) {

    MATCHES *newMatch = malloc(sizeof *newMatch);
    newMatch->next = NULL;
    newMatch->match = new_path; // colocar o match na string

    THREADS *newThread = malloc(sizeof *newThread);
    newThread->thread_id = self;
    newThread->next = NULL;
    newThread->n_matches = 1;
    newThread->matches = newMatch; // the head of the matches

    pthread_mutex_lock(&trinco);
    n_threads++;
    // no caso de ainda nao ter sido criada nenhuma estrutura esta primeira ficara assigned a variavel global head
    if (head == NULL) {
        head = newThread;

    } else {
        THREADS *current = head;
        THREADS *next = current->next;

        // procura a ultima posicao da linked list
        while (next != NULL) {
            current = next;
            next = current->next;
        }
        current->next = newThread; // proxima posicao NULL igual ao novo thread
    }
    pthread_mutex_unlock(&trinco);

    return newThread;
}

MATCHES *create_new_match_in_thread(MATCHES *current_match, char *new_path) {
    MATCHES *match = malloc(sizeof *match);
    match->next = NULL;
    match->match = new_path;
    current_match->next = match;
    return match;
}

//consome tudo dentro deste path (directorio)
void consome(char *path_of_search, DATA this_data) {
    int j;

    struct threads *newThread = NULL; //para o caso da thread ainda nao existir se ja existir procuro o coloco nesta variavel
    struct matches *newMatch = NULL; //para o caso de ter um match ou mais
    int n_matches = 0;
    DIR *dir;
    struct dirent *entry;


    if ((dir = opendir(path_of_search)) == NULL) {
        perror("opendir() error");
        printf("Path : %s\n", path_of_search);
    } else {
        // enqyanto os directorios de dir nao forem null
        while ((entry = readdir(dir)) != NULL) {

            if (strcmp(entry->d_name, "..") != 0 && strcmp(entry->d_name, ".") != 0 &&
                strcmp(entry->d_name, "") != 0) {

                char *name_of_file = (char *) malloc(strlen(entry->d_name));
                strcpy(name_of_file, (char *) entry->d_name);

                char *new_path = malloc(300);
                if (path_of_search[strlen(path_of_search) - 1] != '/') {
                    sprintf(new_path, "%s/%s", path_of_search,
                            name_of_file);
                } else {
                    new_path = strcat(path_of_search, name_of_file);
                }

                //chama as funcoes de possivel match de acordo com os argumentos definidos
                for (j = 0; j < this_data.n_args; j++) {
                    if (!this_data.args[j].opt(this_data.args[j].value, new_path, name_of_file)) {
                        break;
                    }
                }
                // Match
                 if (j == this_data.n_args) { // se todas as funcoes passarem nos testes j sera igual ao numero de argumentos definidos
                    if (n_matches == 0) {

                        // retorna a struct existente se ela existir, cria strcuct match se ela nao existir
                        // se ela nao existir cria a e cria a strcut match dando os devidos paramnetros
                        newThread = create_thread_and_match(pthread_self(), new_path);
                        newMatch = newThread->matches;

                    } else {

                        // neste caso ja temos as structs criadas
                        //temos de ir a ultima posicao de match para criar outra struct match
                        newMatch = create_new_match_in_thread(newMatch, new_path);
                        //atualiza numero de matches
                        newThread->n_matches++;

                    }
                    n_matches++;
                }
            }
        }
    }
}

void produce_new_path_to_consume(char * new_path){
    sem_wait(&semPodeProd);            // espera que o semaforo baixe para poder produzir
    pthread_mutex_lock(&trinco_p);        // apanha o lock para entrar na regiao critica
    data[prodptr].path = malloc(300);
    strcpy(data[prodptr].path, new_path);
    prodptr = (prodptr + 1) % N;            //atualiuza o prodptr para a proxima posicao
    pthread_mutex_unlock(&trinco_p);    // liberta o lock para outros produtores
    sem_post(&semPodeCons);            // aumenta o semaforo de produtores

}

void *produtor(void *param) {

    char *path_to_explore = (char *) param;

    // retiro o path a procurar nesta thread
    char *path_of_search = malloc(300); // 300 for the record
    strcpy(path_of_search, path_to_explore);


    DIR *dir;
    struct dirent *entry;

    // abre directorio para explorar contreudo de primeiro nivel
    if ((dir = opendir(path_of_search)) == NULL) {
        perror("opendir() error");
        printf("Path : %s\n", path_of_search);
    } else {
        // enqyanto os directorios de dir nao forem null
        while ((entry = readdir(dir)) != NULL) {

            if (strcmp(entry->d_name, "..") != 0 && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "") != 0) {

                char *name_of_file = (char *) malloc(50);
                strcpy(name_of_file, (char *) entry->d_name);

                char *new_path = malloc(300);
                if (path_of_search[strlen(path_of_search) - 1] != '/') {
                    sprintf(new_path, "%s/%s", path_of_search,
                            name_of_file);
                } else {
                    new_path = strcat(path_of_search, name_of_file);
                }

                //printf("new path = %s\n",new_path);
                // se for directorio cria outro thread para o explorar
                if (isDirectory(new_path)) {
                    produce_new_path_to_consume(new_path); // adiciona ao array global uma nova estrtura para consumir e atualiza as variaveis
                    produtor(new_path);
                }
            }
        }
    }
    closedir(dir);
    return NULL;
}


void *consumidor() {
    while (1) {
        sem_wait(&semPodeCons);             // espera por ser sinalizado para poder consumir
            pthread_mutex_lock(&trinco_c);      // aguarda o lock da regiao critica
                char *path_of_search = malloc(300); // aloca espaco para path a consumir
                int n = consptr;
                strcpy(path_of_search, data[consptr].path); // copia o path para local da estrtura global dentro de locks
                data[consptr].path = NULL;                  // coloca a NULL o path consumido
                consptr = (consptr + 1) % N;        // coloca o consptr para a proxima posicao, se for a ultima posicap passa para a primeira devido ao %N
            pthread_mutex_unlock(&trinco_c);    // liberta o lock para outras threads consumidoras entrarem na regiao critica
        sem_post(&semPodeProd);             // aumenta o semaforo de consumidores

        consome(path_of_search, data[n]); // explora os ficheiro dentro de path_pf_search e encontra matches
    }
}

void print_matches(THREADS *threads, char * base_path) {
    if (threads == NULL) {
        printf("\tNao existem resultados para a sua pesquisa\n");
    } else {
        THREADS *tmp = threads; // tmp fica com a primeria thrad inicializada
        for (int i = 0; i < n_threads; i++) {
            MATCHES *mat = tmp->matches; // tmp fica com a primeria thrad inicializada
            for (int j = 0; j < tmp->n_matches; j++) {
                char *str = strremove(mat->match,base_path); // remove o base_path do path completo para ficar mais bonito
                printf(".%s\n", str);
                mat = mat->next;
            }
            tmp = tmp->next; // tmp fica com a thread inicializada a seguir a mesma
        }
    }
}


int main(int argc, char *argv[]) {
    //nesta funcao prodptr comeca a 0

    pthread_t thread_producer_id[NProds]; // thread ids dos produtores (1)
    pthread_t thread_consumer_id[NProds]; // thread ids dos consumidores

    // movemos a informacao para a primeira posicao do array de structs data
    parse_args(argc, argv,&data[prodptr]); // faz o parse dos argumentos recebidos do terminal e coloca os dentro da estrtura
    if (data[prodptr].n_args > 0) {
        // copiamos a informacao passada por argumento da primeira struct do array para as outras structs
        for (int j = 1; j < N; j++) {
            for (int i = 0; i < data[prodptr].n_args ; i++){
                data[j].args[i].opt = data[prodptr].args[i].opt;
                data[j].args[i].value = data[prodptr].args[i].value;
            }
            data[j].n_args = data[prodptr].n_args;
        }
    }
    // inicicializamos o semaforo com um produtor e N consumidores
    sem_init(&semPodeProd, 0, N); // posicoes que o consumidor pode consumir
    sem_init(&semPodeCons, 0, 0);


    char * base_path = malloc(300);
    strcpy(base_path,data[prodptr].path);

    // criacao de thread produtora
    for (int i = 0; i < NProds; i++) {
        pthread_create(&thread_producer_id[i], NULL, &produtor, (void *) data[prodptr].path);
    }

    // criacao de threads consumidoras
    for (int i = 1; i < NCons + 1; i++) {
        pthread_create(&thread_consumer_id[i], NULL, &consumidor, NULL);
    }

    // o base path sera consumido e nao so os ficheiros incluidos no mesmo
    if (isDirectory(base_path)) {
        produce_new_path_to_consume(base_path);
    }

    // check if base path is match




    

    // faz join da thread consumidora
    pthread_join(thread_producer_id[0], NULL); // espera que thread produtora acabe a recursividade
    // depois da thread produtora ter terminado fica a espera que todas as threads consumidoras consumam
    while (1) {
        int n_consumers_waitting = 0;
        for (int k = 0; k < NCons; k++) {
            if (data[k].path == NULL) {
                n_consumers_waitting++;
            }
        }
        // se todas as threads consumidoras ewstiverem presas a espera de receber algo para consumir,
        // que nunca vai existir visto que a thread produtora ja terminou
        if (n_consumers_waitting == NCons) {
            for (int i = 0; i < NCons; i++) {
                pthread_detach(thread_consumer_id[i]); // faz detach das threads
            }
            print_matches(head,base_path); // imprime os matches
            exit(EXIT_SUCCESS); // exit from main thread
        }
    }

}
