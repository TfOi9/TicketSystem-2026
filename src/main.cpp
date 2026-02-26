#include "../include/system/ticket.hpp"

#include <csignal>

volatile std::sig_atomic_t status = 0;

void signal_handler(int sig) {
    if (sig == SIGINT) {
        std::cerr << " **captured SIGINT signal** " << std::endl;
        status = SIGINT;
    }
    else if (sig == SIGTERM) {
        std::cerr << " **captured SIGTERM signal** " << std::endl;
        status = SIGTERM;
    }
}

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    sjtu::TicketSystem sys;
    sys.run(&status);
    return 0;
}