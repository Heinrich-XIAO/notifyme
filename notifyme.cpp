#include <iostream>
#include <string>
#include <cstdio> 
#include <array>
#include <thread>
#include <chrono>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <sys/wait.h>  
#endif

void sendNotification(const std::string &title, const std::string &content) {
    pid_t pid = fork();

    if (pid == 0) {
        const char *args[] = {"/usr/bin/notify-send", title.c_str(), content.c_str(), nullptr};

        execv("/usr/bin/notify-send", const_cast<char *const *>(args));

        std::cerr << "Error: execv failed to send notification!" << std::endl;
        _exit(1); 
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0); 
    } else {
        std::cerr << "Error: Failed to fork a new process!" << std::endl;
    }
}

std::pair<std::string, std::string> runJavaScriptAndCaptureOutput() {
	std::array<char, 128> buffer;
	std::string title, content;

	FILE* pipe = popen("node condition.js", "r");
	if (!pipe) {
		std::cerr << "Error: Could not start JavaScript process!" << std::endl;
		return {"", ""};
	}

	if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
		title = buffer.data();
		title.erase(title.find_last_not_of("\n\r") + 1); 
	}

	if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
		content = buffer.data();
		content.erase(content.find_last_not_of("\n\r") + 1); 
	}

	pclose(pipe);

	return {title, content};
}

int main() {
	while (true) {
		auto [title, content] = runJavaScriptAndCaptureOutput();

		if (!title.empty() && !content.empty()) {
			sendNotification(title, content);
		}

		std::this_thread::sleep_for(std::chrono::seconds(60));
	}
	return 0;
}

