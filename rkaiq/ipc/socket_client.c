#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <stddef.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/socket.h>
#include "socket_client.h"
#include "xcam_log.h"

#define USE_IPC_SERVER

#ifdef USE_IPC_SERVER
#define LOCALSOCKET_NAME "/tmp/UNIX.domain0"
#else
#ifdef __ANDROID__
#define LOCALSOCKET_NAME "/dev/socket/camera_tool"
#else
#define LOCALSOCKET_NAME "/tmp/camera_tool"
#endif
#endif

#define BUF_SIZE 1024

#define FD_SET_MAX(fd, max, set) do { \
    FD_SET(fd, set); \
    if (max < fd + 1) max = fd + 1; \
} while (0)


static void recv_process(SocketClientCtx_t *ctx)
{
    int maxfd;
    int ret;
    int cid = ctx->cid;
    fd_set rset, wset, xset;

    FD_ZERO(&rset);
    FD_ZERO(&wset);
    FD_ZERO(&xset);
    maxfd = 0;

    FD_SET_MAX(ctx->stopfd[0], maxfd, &rset);
    FD_SET_MAX(ctx->data_fd, maxfd, &rset);

    LOGD_IPC("%s[%d] start select, maxfd %d ...", __func__, cid, maxfd);
    ret = select(maxfd, &rset, NULL, NULL, NULL);
    if (ret >= 0) {
        if (FD_ISSET(ctx->stopfd[0], &rset)) {
            char c;
            ssize_t ret = read(ctx->stopfd[0], &c, sizeof(c));
            ctx->quit = true;
            LOGD_IPC("%s[%d]: stop signal read %x ...", __func__, cid, c);
            return;
        }
        if (FD_ISSET(ctx->data_fd, &rset)) {
            ret = recv(ctx->data_fd, ctx->recvbuf, BUF_SIZE, 0);
            if (ret <= 0) {
                LOGI_IPC("%s[%d]: toolbridge closed!", cid, __func__);
                close(ctx->data_fd);
                ctx->data_fd = 0;
                ctx->connected = false;
            } else {
                message_receiver_gotdata(&ctx->mReceiver, (uint8_t *)ctx->recvbuf, ret);
            }
        }
    }
}

static void sleep_with_select(SocketClientCtx_t *ctx, int sec) {
    fd_set rset;
    int maxfd = 0;
    struct timeval tv = {0};
    tv.tv_sec = sec;

    FD_ZERO(&rset);
    FD_SET_MAX(ctx->stopfd[0], maxfd, &rset);
    int ret = select(maxfd, &rset, NULL, NULL, &tv);
    if (ret >= 0) {
        if (FD_ISSET(ctx->stopfd[0], &rset)) {
            char c;
            ssize_t ret = read(ctx->stopfd[0], &c, sizeof(c));
            ctx->quit = true;
            return;
        }
    }
}

static void try_connect(SocketClientCtx_t *ctx, struct sockaddr_un *addr, int alen)
{
    int cid = ctx->cid;

    LOGD_IPC("%s[%d]: try connect ...", __func__, cid);
    if(connect(ctx->sockfd, (struct sockaddr *) addr, alen) >= 0) {
        LOGI_IPC("%s[%d]: toolbridge connected!", __func__, cid);
        ctx->data_fd = ctx->sockfd;
        ctx->mReceiver.send_fd = ctx->data_fd;
        ctx->connected = true;
        return;
    }

    // wait for 3 seconds
    sleep_with_select(ctx, 3);
}

static void try_accept(SocketClientCtx_t *ctx, struct sockaddr_un *addr, int alen)
{
    int cid = ctx->cid;
    LOGD_IPC("%s[%d]: try accept %d ...", __func__, cid, ctx->sockfd);

    int connfd = accept(ctx->sockfd, NULL, NULL);
    if (connfd > 0) {
        LOGI_IPC("%s[%d]: accept connfd %d...", __func__, cid, connfd);
        ctx->data_fd = connfd;
        ctx->mReceiver.send_fd = ctx->data_fd;
        ctx->connected = true;
        return;
    }

    // wait for 3 seconds
    sleep_with_select(ctx, 3);
}

static void *ClientThreadFunc(void *p)
{
    SocketClientCtx_t *ctx = (SocketClientCtx_t *)p;
    int ret;
    int cid = ctx->cid;

    struct sockaddr_un addr;
    size_t namelen;
    socklen_t alen;
    memset (&addr, 0, sizeof (addr));

    /* unix_path_max appears to be missing on linux */
    namelen = strlen(LOCALSOCKET_NAME);
    addr.sun_family = AF_LOCAL;
    strcpy(addr.sun_path, LOCALSOCKET_NAME);
    alen = namelen + offsetof(struct sockaddr_un, sun_path) + 1;
    LOGD_IPC("%s[%d]: local client sun_path %s, len %d", __func__, cid, addr.sun_path, namelen);

    ctx->sockfd = socket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
    if (ctx->sockfd <= 0) {
        LOGE_IPC("%s[%d]: create socket error: %s", strerror(errno), __func__, cid);
        return NULL;
    }
    LOGI_IPC("%s[%d]: create sockfd %d...", __func__, cid, ctx->sockfd);

#ifdef USE_IPC_SERVER
    int n = 1;
    /*ignore ENOENT*/
    unlink(addr.sun_path);
    setsockopt(ctx->sockfd, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n));

    if (bind(ctx->sockfd, (struct sockaddr *) &addr, alen) != 0) {
        LOGE_IPC("bind local server socket error!");
        return NULL;
    }
    LOGD_IPC("bind local server socket ok ...");
    if (listen(ctx->sockfd, 2) != 0) {
        LOGE_IPC("listen local server socket error!");
        return NULL;
    }
    LOGD_IPC("listen local server socket ok ...");
#endif

    ctx->thread_running = true;
    while (!ctx->quit) {
        if (!ctx->connected) {
#ifdef USE_IPC_SERVER
            try_accept(ctx, &addr, alen);
#else
            try_connect(ctx, &addr, alen);
#endif
        } else {
            recv_process(ctx);
        }
    }
    ctx->thread_running = false;
    return NULL;
}

int socket_client_start(void *aiqctx, SocketClientCtx_t *ctx, int cid) {
    int ret;
    LOGD_IPC("%s[%d]: enter", __func__, cid);

    ctx->enable = true;
    ctx->connected = false;
    ctx->quit = false;
    ctx->thread_running = false;
    ctx->sockfd = 0;
    ctx->cid = cid;
    ctx->aiqctx = aiqctx;

    message_receiver_init(&ctx->mReceiver);
    ctx->mReceiver.aiqctx = aiqctx;

    ctx->recvbuf = malloc(BUF_SIZE);
    if (ctx->recvbuf == NULL) {
        LOGE_IPC("%s[%d]: malloc recvbuf error", __func__, cid);
        return -1;
    }

    if (pipe(ctx->stopfd) < 0) {
        LOGE_IPC("%s[%d]: create stop pipe error: %s", __func__, cid, strerror(errno));
        return -1;
    }

    ret = pthread_create(&ctx->client_thread, NULL, ClientThreadFunc, ctx);
    if (ret != 0) {
        LOGE_IPC("%s[%d]: create ipc thread error: %s", __func__, cid, strerror(ret));
        return -1;
    }

    LOGD_IPC("%s[%d]: done",__func__, cid);
    return 0;
}

void socket_client_exit(SocketClientCtx_t *ctx) {
    int cid = ctx->cid;
    LOGD_IPC("%s[%d]: enter", __func__, cid);

    if (ctx->thread_running) {
        char c = 0xfe;
        ctx->quit = true;
        ssize_t size = write(ctx->stopfd[1], &c, sizeof(c));
        pthread_join(ctx->client_thread, NULL);
    }

    if (ctx->recvbuf)
        free(ctx->recvbuf);
    if (ctx->sockfd)
        close(ctx->sockfd);
    if (ctx->stopfd[0])
        close(ctx->stopfd[0]);
    if (ctx->stopfd[1])
        close(ctx->stopfd[1]);

    message_receiver_deinit(&ctx->mReceiver);
    LOGD_IPC("%s[%d]: done", __func__, cid);
}

void socket_client_setNote(SocketClientCtx_t *ctx, uint32_t ret, char *str)
{
    if (ctx == NULL)
        return;
    receiver_t *rec = &ctx->mReceiver;
    if(rec->ret == 0xff00)
        rec->ret = ret;

    if (rec->note[0] == 0) {
        if (str)
            strcpy(rec->note, str);
    }
}

bool socket_client_getConnected(SocketClientCtx_t *ctx)
{
    if (ctx == NULL)
        return false;

    return ctx->connected;
}

void socket_client_setEnable(SocketClientCtx_t *ctx, bool enable)
{
    if (ctx == NULL)
        return;
    ctx->enable = enable;
}

