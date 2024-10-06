#include <iostream>
#include <string>
#include <cstdio> 
#include <array>
#include <thread>
#include <chrono>
#include <fstream>
#include <cstring>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <sstream>
#endif

std::string runCommand(const std::string& command, const std::vector<std::string>& args) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        std::cerr << "Pipe failed!" << std::endl;
        return "";
    }

    pid_t pid = fork();
    
    if (pid < 0) {
        std::cerr << "Fork failed!" << std::endl;
        return "";
    }

    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(command.c_str()));
        for (const auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);
        execvp(argv[0], argv.data());
        std::cerr << "execvp failed!" << std::endl;
        exit(EXIT_FAILURE);
    } else {
        close(pipefd[1]);
        std::stringstream output;
        char buffer[128];
        ssize_t bytesRead;

        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            output << buffer;
        }
        close(pipefd[0]);
        int status;
        waitpid(pid, &status, 0);
        
        if (!WIFEXITED(status)) {
            std::cerr << "Child process did not exit normally!" << std::endl;
        }

        return output.str();
    }
}

void sendNotification(const std::string &title, const std::string &content) {
	runCommand("/usr/bin/notify-send", {title, content});
}

std::pair<std::string, std::string> checkCondition(std::string condition_file) {
	std::string output = runCommand("node", {condition_file});
	if (output.empty()) {
		return {"", ""};
	}

	std::string title = output.substr(0, output.find('\n'));
	std::string content = output.substr(output.find('\n') + 1);

	return {title, content};
}

int main(int argc, char *argv[]) {
	if (argc != 1) {
		if (argc == 3 && strcmp(argv[1], "add") == 0) {
			std::ofstream file("conditions.txt", std::ios_base::app);
			if (!file.is_open()) {
				std::cerr << "Error: Could not open conditions.txt!" << std::endl;
				return 1;
			}
			file << argv[2] << std::endl;
			file.close();
			return 0;
		}
	}


	std::ifstream file("conditions.txt");
	if (!file.is_open()) {
		std::cerr << "Error: Could not open conditions.txt!" << std::endl;
		return 1;
	}

	std::vector<std::string> condition_files;
	std::string line;

	while (file >> line) {
		condition_files.push_back(line);
	}
	file.close();

	while (true) {
		for (const auto& condition_file : condition_files) {
			auto [title, content] = checkCondition(condition_file);
			if (!title.empty() && !content.empty()) {
				sendNotification(title, content);
			}
		}

		std::this_thread::sleep_for(std::chrono::seconds(60));
	}
	return 0;
}

