#pragma


class SocketListener
{
    public:
        SocketListener(const char *ip, int port);

        int fd() const;
    
    private:
        int fd_;
};