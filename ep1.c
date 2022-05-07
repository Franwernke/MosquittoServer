/* Por Francisco Eugênio Wernke (NUSP: 11221870)
** Com base no código do enunciado elaborado pelo Professor Daniel Batista
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>

/** Para usar o mkfifo() **/
#include <sys/stat.h>
/** Para usar o open e conseguir abrir o pipe **/
#include <fcntl.h>
/** Para usar threads **/
#include <pthread.h>

#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096
#define BASEPIPEPATH "/tmp/ep1/"

enum operations {
    CONNECT = 0x10,
    CONNACK = 0x20,
    PUBLISH = 0x30,
    PUBACK = 0x40,
    SUBSCRIBE = 0x80,
    SUBACK = 0x90,
    UNSUBSCRIBE = 0xa0,
    UNSUBACK = 0xb0,
    DISCONNECT = 0xe0,
    PINGREQ = 0xc0,
    PINGRESP = 0xd0
};

typedef struct {
    char *topicPath;
    int connfd;
} TopicArgs;

void sendConnack(int connfd) {
    char connackMessage[4] = {0x20, 0x02, 0x00, 0x00};
    printf("server CONNACK\n");
    write(connfd, connackMessage, 4);
}

void sendSuback(int connfd) {
    char subackMessage[5] = {0x90, 0x03, 0x00, 0x01, 0x00};
    printf("server SUBACK\n");
    write(connfd, subackMessage, 5);
}

void sendPingresp(int connfd) {
    char pingrespMessage[2] = {0xd0, 0x00};
    printf("server PINGRESP\n");
    write(connfd, pingrespMessage, 2);
}

char * retrieveTopicFromPublish(char *recvline) {
    int sizeTopicName = recvline[2] + recvline[3];
    char *topic = malloc(sizeTopicName);
    strncpy(topic, recvline + 4, sizeTopicName);
    topic[sizeTopicName] = 0;
    return topic;
}

char * retrieveTopicFromSubscribe(char *recvline) {
    int sizeTopicName = recvline[4] + recvline[5];
    char *topic = malloc(sizeTopicName);
    
    for (int i = 0, j = 6; i < sizeTopicName; i++, j++) {
        topic[i] = recvline[j];
    }
    topic[sizeTopicName] = 0;
    return topic;
}

char * retrieveMessage(char *recvline) {
    int sizeTopicName = recvline[2] + recvline[3];
    int sizeMessageName = recvline[1] - sizeTopicName - 2;
    char *message = malloc(sizeMessageName + 1);
    
    for (int i = 0, j = sizeTopicName + 4; i < sizeMessageName; i++, j++) {
        message[i] = recvline[j];
    }
    return message;
}

void publishToTopic(char *topicName, char *message) {
    DIR *directoryPointer;
    struct dirent *dir;
    char *topicDir = malloc(strlen(BASEPIPEPATH) + strlen(topicName) + 1);

    strcpy(topicDir, BASEPIPEPATH);
    strcat(topicDir, topicName);

    directoryPointer = opendir(topicDir);
    if (directoryPointer) {
        while ((dir = readdir(directoryPointer)) != NULL) {
            if (dir->d_type != DT_FIFO) continue;

            char *topicPath = malloc(strlen(topicDir) + 9);
            strcpy(topicPath, topicDir);
            strcat(topicPath, "/");
            strcat(topicPath, dir->d_name);
            int fd = open(topicPath, O_WRONLY);
            write(fd, message, strlen(message));
            close(fd);
            free(topicPath);
        }
        closedir(directoryPointer);
    }

    free(topicDir);
    free(topicName);
    free(message);
}

char * createTopic(char *topicName) {
    char *pipeTemplate = malloc(strlen(BASEPIPEPATH) + strlen(topicName) + 8);
    char *pipeDir = malloc(strlen(BASEPIPEPATH) + strlen(topicName) + 2);
    struct stat st = {0};

    if (stat(BASEPIPEPATH, &st) == -1) {
        if (mkdir(BASEPIPEPATH, 0777)) {
            perror("mkdir ep1 :(");
            exit(1);
        }
    }

    strcpy(pipeDir, BASEPIPEPATH);
    strcat(pipeDir, topicName);
    strncat(pipeDir, "/", 1);

    if (stat(pipeDir, &st) == -1) {
        if (mkdir(pipeDir, 0777)) {
            perror("mkdir :(");
            exit(1);
        }
    }

    strcpy(pipeTemplate, pipeDir);
    strncat(pipeTemplate, "XXXXXX", 6);

    char *pipePath = malloc(strlen(pipeTemplate) + 1);
    char *topicPath = malloc(strlen(pipeTemplate) + 1);
    pipePath = mktemp(pipeTemplate);
    strcpy(topicPath, pipePath);
    if (mkfifo(pipePath, 0644)) {
        perror("mkfifo :(\n");
        exit(1);
    }
    free(pipeTemplate);
    free(pipeDir);

    return topicPath;
}

char *makePublishPacket(int messageSize, char *buffer, char *topicPath) {
    char *topicNameSub = malloc(strlen(topicPath) - strlen(BASEPIPEPATH) - 6);
    strncpy(topicNameSub, topicPath + strlen(BASEPIPEPATH), strlen(topicPath) - strlen(BASEPIPEPATH) - 7);
    int packetRemainingSize = 2 + strlen(topicNameSub) + messageSize;
    char *publishPacket = malloc(2 + packetRemainingSize);

    publishPacket[0] = PUBLISH;
    publishPacket[1] = (char)packetRemainingSize;
    publishPacket[2] = (char)strlen(topicNameSub) >> 8;
    publishPacket[3] = (char)strlen(topicNameSub);
    for (int i = 0, j = 4; i < strlen(topicNameSub); i++, j++) {
        publishPacket[j] = topicNameSub[i];
    }
    for (int i = 0, j = 4 + strlen(topicNameSub); i < messageSize; i++, j++) {
        publishPacket[j] = buffer[i];
    }
    return publishPacket;
}

int disconnected = 0;

void *readFromTopic(void *args) {
    TopicArgs *topicArgs = *(TopicArgs **)args;
    char *topicPath = topicArgs->topicPath;
    int connfd = topicArgs->connfd;

    int fd;
    char buffer[MAXLINE + 1];
    int messageSize;
    while ((fd = open(topicPath, O_RDONLY)) && (messageSize = read(fd, buffer, MAXLINE)) > 0 && !disconnected) {
        char *publishPacket = makePublishPacket(messageSize, buffer, topicPath);
        write(connfd, publishPacket, publishPacket[1] + 2);
        close(fd);
    }
    pthread_exit(NULL);
}

void *readFromConnfd(void *connfdP) {
    int connfd = *(int *)connfdP;
    int n;
    char recvline[MAXLINE + 1];
    while(!disconnected && (n = read(connfd, recvline, MAXLINE)) > 0) {
        enum operations currentOperation = (recvline[0] & 0xf0);

        switch (currentOperation) {
            case PINGREQ:
                sendPingresp(connfd);
            break;
            case DISCONNECT:
                disconnected = 1;
            break;
            default:
                printf("Forbidden operation %02x received from client\n", currentOperation);
                exit(1);
        }
    }
    pthread_exit(NULL);
}

void waitForMessages(int connfd, char *topicPath) {
    pthread_t *tids = malloc(sizeof(pthread_t) * 2);
    TopicArgs *args = malloc(sizeof(TopicArgs));
    args->connfd = connfd;
    args->topicPath = topicPath;
    disconnected = 0;

    if (pthread_create(&tids[0], NULL, readFromTopic, &args)) {
        perror("Create readFromTopic thread :(");
        exit(1);
    }
    if (pthread_create(&tids[1], NULL, readFromConnfd, &connfd)) {
        perror("Create readFromConnfd thread :(");
        exit(1);
    }
    if (pthread_join(tids[1], NULL)) {
        perror("join thread readFromConnfd :(");
        exit(1);
    }
    if (pthread_cancel(tids[0])) {
        perror("cancel thread readFromTopic :(");
        exit(1);
    }
    unlink(topicPath);
    free(tids);
    free(args->topicPath);
    free(args);
    printf("client disconnected\n");
}

int main (int argc, char **argv) {
    /* Os sockets. Um que será o socket que vai escutar pelas conexões
     * e o outro que vai ser o socket específico de cada conexão */
    int listenfd, connfd;
    /* Informações sobre o socket (endereço e porta) ficam nesta struct */
    struct sockaddr_in servaddr;
    struct sockaddr_in clientaddr;
    /* Retorno da função fork para saber quem é o processo filho e
     * quem é o processo pai */
    pid_t childpid;
    /* Armazena linhas recebidas do cliente */
    char recvline[MAXLINE + 1];
    /* Armazena o tamanho da string lida do cliente */
    ssize_t n;
    socklen_t clientaddr_size = sizeof(clientaddr);
 
    if (argc != 2) {
        fprintf(stderr,"Uso: %s <Porta>\n",argv[0]);
        fprintf(stderr,"Vai rodar um servidor de echo na porta <Porta> TCP\n");
        exit(1);
    }

    /* Criação de um socket. É como se fosse um descritor de arquivo.
     * É possível fazer operações como read, write e close. Neste caso o
     * socket criado é um socket IPv4 (por causa do AF_INET), que vai
     * usar TCP (por causa do SOCK_STREAM), já que o MQTT funciona sobre
     * TCP, e será usado para uma aplicação convencional sobre a Internet
     * (por causa do número 0) */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket :(\n");
        exit(2);
    }

    /* Agora é necessário informar os endereços associados a este
     * socket. É necessário informar o endereço / interface e a porta,
     * pois mais adiante o socket ficará esperando conexões nesta porta
     * e neste(s) endereços. Para isso é necessário preencher a struct
     * servaddr. É necessário colocar lá o tipo de socket (No nosso
     * caso AF_INET porque é IPv4), em qual endereço / interface serão
     * esperadas conexões (Neste caso em qualquer uma -- INADDR_ANY) e
     * qual a porta. Neste caso será a porta que foi passada como
     * argumento no shell (atoi(argv[1]))
     */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(atoi(argv[1]));
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind :(\n");
        exit(3);
    }

    /* Como este código é o código de um servidor, o socket será um
     * socket passivo. Para isto é necessário chamar a função listen
     * que define que este é um socket de servidor que ficará esperando
     * por conexões nos endereços definidos na função bind. */
    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen :(\n");
        exit(4);
    }

    printf("[Servidor no ar. Aguardando conexões na porta %s]\n",argv[1]);
    printf("[Para finalizar, pressione CTRL+c ou rode um kill ou killall]\n");
   
    /* O servidor no final das contas é um loop infinito de espera por
     * conexões e processamento de cada uma individualmente */
	for (;;) {
        /* O socket inicial que foi criado é o socket que vai aguardar
         * pela conexão na porta especificada. Mas pode ser que existam
         * diversos clientes conectando no servidor. Por isso deve-se
         * utilizar a função accept. Esta função vai retirar uma conexão
         * da fila de conexões que foram aceitas no socket listenfd e
         * vai criar um socket específico para esta conexão. O descritor
         * deste novo socket é o retorno da função accept. */
        if ((connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientaddr_size)) == -1) {
            perror("accept :(\n");
            exit(5);
        }
      
        /* Agora o servidor precisa tratar este cliente de forma
         * separada. Para isto é criado um processo filho usando a
         * função fork. O processo vai ser uma cópia deste. Depois da
         * função fork, os dois processos (pai e filho) estarão no mesmo
         * ponto do código, mas cada um terá um PID diferente. Assim é
         * possível diferenciar o que cada processo terá que fazer. O
         * filho tem que processar a requisição do cliente. O pai tem
         * que voltar no loop para continuar aceitando novas conexões.
         * Se o retorno da função fork for zero, é porque está no
         * processo filho. */
        if ( (childpid = fork()) == 0) {
            /**** PROCESSO FILHO ****/
            printf("[Uma conexão aberta]\n");
            /* Já que está no processo filho, não precisa mais do socket
             * listenfd. Só o processo pai precisa deste socket. */
            close(listenfd);
         
            /* Agora pode ler do socket e escrever no socket. Isto tem
             * que ser feito em sincronia com o cliente. Não faz sentido
             * ler sem ter o que ler. Ou seja, neste caso está sendo
             * considerado que o cliente vai enviar algo para o servidor.
             * O servidor vai processar o que tiver sido enviado e vai
             * enviar uma resposta para o cliente (Que precisará estar
             * esperando por esta resposta) 
             */

            /* ========================================================= */
            /* ========================================================= */
            /*                         EP1 INÍCIO                        */
            /* ========================================================= */
            /* ========================================================= */
            while((n = read(connfd, recvline, MAXLINE)) > 0) {
                enum operations currentOperation = (recvline[0] & 0xf0);
                
                switch (currentOperation) {
                case CONNECT:
                    sendConnack(connfd);
                    break;
                case PUBLISH:
                    printf("client PUBLISH\n");
                    char *topicNamePub = retrieveTopicFromPublish(recvline);
                    char *message = retrieveMessage(recvline);
                    publishToTopic(topicNamePub, message);
                    break;
                case SUBSCRIBE:
                    printf("********client SUBSCRIBE\n");
                    char *topicNameSub = retrieveTopicFromSubscribe(recvline);
                    char *topicPath = createTopic(topicNameSub);
                    sendSuback(connfd);
                    waitForMessages(connfd, topicPath);
                    break;
                case DISCONNECT:
                    printf("client DISCONNECT\n");
                    break;
                default:
                    printf("client unknown operation\n");
                    printf("Operation: %02x\n", (recvline[0] & 0xf0));
                }
            }

            /* ========================================================= */
            /* ========================================================= */
            /*                         EP1 FIM                           */
            /* ========================================================= */
            /* ========================================================= */

            /* Após ter feito toda a troca de informação com o cliente,
             * pode finalizar o processo filho */
            printf("[Uma conexão fechada]\n");
            exit(0);
        }
        else
            /**** PROCESSO PAI ****/
            /* Se for o pai, a única coisa a ser feita é fechar o socket
             * connfd (ele é o socket do cliente específico que será tratado
             * pelo processo filho) */
            close(connfd);
    }
    exit(0);
}
