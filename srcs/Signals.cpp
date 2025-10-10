#include "Signals.hpp"

volatile bool serverRunning = true;

void handleSignal(int signum)
{
	if (signum == SIGINT)
	{
		std::cout << "\n\033[1;31m[!] Caught SIGINT — shutting down cleanly...\033[0m\n";
		serverRunning = false;
	}
}
