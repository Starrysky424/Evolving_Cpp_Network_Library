#pragma

#include <sys/select.h>
#include<vector>
class SelectPoller
{
public:
    SelectPoller();

    // 加入监听集合
    void add_fd(int fd);

    // 移出监听集合
    void remove_fd(int fd);

    int poll(int timeout_sec);

    

    const std::vector<int> &get_ready_fds() const;

private:
    fd_set master_fds_;
    fd_set ready_fds_;
    int max_fd_;
    std::vector<int> ready_fds_list_;
};