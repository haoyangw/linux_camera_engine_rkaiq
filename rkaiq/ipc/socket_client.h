#ifndef _SOCKET_CLIENT_H_
#define _SOCKET_CLIENT_H_

#include <pthread.h>
#include <stdbool.h>
#include "message_receiver.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define IPC_RET_OK  0xff00
#define IPC_RET_JSON_ERROR  0xff01
#define IPC_RET_UAPI_ERROR  0xff02
#define IPC_RET_AIQ_ERROR  0xff03
#define IPC_RET_UAPI_WARNING  0xff04

typedef struct {
    int cid;
    int sockfd;
    int client_fd;
    int data_fd;
    int stopfd[2];
    bool quit;
    bool connected;
    bool thread_running;
    pthread_t client_thread;
    char *recvbuf;
    void *aiqctx;
    bool enable;

    receiver_t mReceiver;
} SocketClientCtx_t;

int socket_client_start(void *aiqctx, SocketClientCtx_t *ctx, int cid);
void socket_client_exit(SocketClientCtx_t *ctx);
void socket_client_setNote(SocketClientCtx_t *ctx, uint32_t ret, char *str);
bool socket_client_getConnected(SocketClientCtx_t *ctx);

#ifdef  __cplusplus
}
#endif

#endif
