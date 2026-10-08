#pragma

#include"Config.h"
class SocketListener
{
    public:
        explicit SocketListener(Config &config);
        ~SocketListener();
        int fd() const;

        int accept();
        void close();

    private:
        int fd_;
        void initSocket(int port, int backlog);

        SocketListener(const SocketListener &) = delete;
        SocketListener &operator=(const SocketListener &) = delete;
};