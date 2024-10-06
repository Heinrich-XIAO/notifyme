#include <iostream>
#include <string>
#include <cstdio>   // For popen() and pclose()
#include <array>
#include <thread>
#include <chrono>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <sys/wait.h>  // For waitpid()
#endif

void sendNotification(const std::string &title, const std::string &content) {
    pid_t pid = fork();  // Create a new process

    if (pid == 0) {
        // Child process
        // Prepare the arguments for execv
        const char *args[] = {"/usr/bin/notify-send", title.c_str(), content.c_str(), nullptr};

        // Replace the child process with the notify-send command
        execv("/usr/bin/notify-send", const_cast<char *const *>(args));

        // If execv fails, print an error and exit the child process
        std::cerr << "Error: execv failed to send notification!" << std::endl;
        _exit(1);  // Exit the child process
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);  // Wait for the child process to finish
    } else {
        // Fork failed
        std::cerr << "Error: Failed to fork a new process!" << std::endl;
    }
}

// Function to run the JavaScript script and capture its output
std::pair<std::string, std::string> runJavaScriptAndCaptureOutput() {
	std::array<char, 128> buffer;
	std::string title, content;

	// Use popen to run the Node.js script and capture the output
	FILE* pipe = popen("node condition.js", "r");
	if (!pipe) {
		std::cerr << "Error: Could not start JavaScript process!" << std::endl;
		return {"", ""};
	}

	// Read the first line (title)
	if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
		title = buffer.data();
		title.erase(title.find_last_not_of("\n\r") + 1);  // Remove trailing newline
	}

	// Read the second line (content)
	if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
		content = buffer.data();
		content.erase(content.find_last_not_of("\n\r") + 1);  // Remove trailing newline
	}

	// Close the pipe
	pclose(pipe);

	return {title, content};
}

int main() {
	while (true) {
		// Run the JavaScript script and capture its output (title and content)
		auto [title, content] = runJavaScriptAndCaptureOutput();

		// If both title and content are non-empty, send the notification
		if (!title.empty() && !content.empty()) {
			sendNotification(title, content);
		}

		std::this_thread::sleep_for(std::chrono::seconds(60));
	}
	return 0;
}

