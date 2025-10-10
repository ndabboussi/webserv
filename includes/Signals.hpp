#ifndef SIGNALS_HPP
# define SIGNALS_HPP

# include <csignal>
# include <iostream>

extern volatile bool serverRunning;

// Signal handler for SIGINT (Ctrl+C)
void handleSignal(int signum);

#endif
