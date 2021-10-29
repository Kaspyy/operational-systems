
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <errno.h>

#define MESSAGE_SIZE 10

key_t key;
int queueId;
int messageSize;

struct Message
{
  long int receiver;
  long int sender;
  char content[MESSAGE_SIZE];
};

void initializeGlobals();
void joinMessageQueue();
void displayMessage(struct Message *message);

void *senderThread();
void *receiverThread();

void initializeGlobals()
{
  key = ftok("keyFile", 10);
  messageSize = MESSAGE_SIZE * sizeof(char) + sizeof(long int);
}

void joinMessageQueue()
{
  queueId = msgget(key, 0600 | IPC_CREAT);
  if (queueId == -1)
  {
    printf("Blad dolaczenia do kolejki o kluczu: %d\n", key);
    exit(1);
  }

  printf("Dolaczono do kolejki o id: %d\n", queueId);
}

void displayMessage(struct Message *message)
{
  printf("Nadawca: %ld\n", message->sender);
  printf("Odbiorca: %ld\n", message->receiver);
  printf("Tresc: %s\n\n", message->content);
}

void receiveMessage(struct Message *message)
{
  if (msgrcv(queueId, message, messageSize, (long int)getpid(), 0) == -1)
  {
    if (errno == EIDRM)
    {
      printf("Blad podczas odbierania komunikatu, kolejka zostala skasowana\n");
      exit(1);
    }

    if (errno == EINTR)
    {
      printf("Blad podczas odbierania komunikatu, wznowienie procesu\n");
      return receiveMessage(message);
    }

    if (errno == EAGAIN)
    {
      printf("Blad podczas odbierania komunikatu, kolejka pelna!\n");
      return;
    }

    printf("Blad podczas odbierania komunikatu, komunikat zbyt duzy zostanie zignorowany\n");
    return;
  }
}

void sendMessage(char *msg)
{
  struct Message message;

  message.receiver = 1;
  message.sender = getpid();
  strcpy(message.content, msg);

  if (msgsnd(queueId, &message, messageSize, 0) == -1)
  {
    if (errno == EIDRM)
    {
      printf("Blad podczas wysylania komunikatu, kolejka zostala skasowana\n");
      exit(1);
    }

    if (errno == EINTR)
    {
      printf("Blad podczas wysylania komunikatu, wznowienie procesu\n");
      return sendMessage(msg);
    }

    if (errno == EAGAIN)
    {
      printf("Blad podczas wysylania komunikatu, kolejka pelna!\n"); // msg_qbytes limit
      return;
    }

    printf("Nie udalo sie wyslac wiadomosci, pelna kolejka?\n");
    exit(1);
  }
}

void *senderThread()
{
  int i;
  char *msg = malloc(MESSAGE_SIZE * sizeof(char));

  i = 0;

  while (1)
  {
    printf("Wysylam wiadomosc nr: %d ...\n", i++);
    printf("Podaj tresc\n");
    scanf("%10s", msg);
    printf("\n");
    sendMessage(msg);

    printf("Wyslano\n");
    //sleep(1);
  }

  free(msg);

  pthread_exit(0);
}

void *receiverThread()
{
  int i;
  struct Message message;

  while (1)
  {
    //printf("przed odebraniem wiadomosci\n");
    receiveMessage(&message);
    //printf("przed wyswietleniem wiadomosci\n");
    displayMessage(&message);
    sleep(1);
  }

  pthread_exit(0);
}

void createThread(pthread_t *threadId, void *threadFunction)
{
  int threadError;

  threadError = pthread_create(threadId, NULL, threadFunction, NULL);
  if (threadError != 0)
    printf("\nNie udalo sie stworzyc watku :[%s]\n", strerror(threadError));
  else
    printf("\nUdalo sie stworzyc watkek\n");
}

void waitThread(pthread_t threadId)
{
  int threadError;
  int **threadStatus;

  threadStatus = malloc(sizeof(*threadStatus));
  *threadStatus = malloc(sizeof(**threadStatus));

  threadError = pthread_join(threadId, (void **)threadStatus);
  if (threadError != 0)
  {
    printf("\nNie udalo sie dolaczyc watku :[%s]", strerror(threadError));
    printf("\nNie udalo sie dolaczyc watku :[%d]", **(int **)threadStatus);
  }
  else
    printf("\nUdalo sie dolaczyc watek\n");

  free(*threadStatus);
  free(threadStatus);
}

int main()
{
  pthread_t senderThreadId;
  pthread_t receiverThreadId;

  initializeGlobals();

  joinMessageQueue();

  createThread(&senderThreadId, &senderThread);
  createThread(&receiverThreadId, &receiverThread);

  waitThread(senderThreadId);
  waitThread(receiverThreadId);

  return 0;
}
