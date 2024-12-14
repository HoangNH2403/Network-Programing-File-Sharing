#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include "command.h"
#include "common.h"
#include "connect.h"

const char *handle_pwd(char *cur_user, char *user_dir) {
    std::vector<std::string> tokens;
    std::string token;
    std::string user_dir_str(user_dir);
    std::istringstream tokenStream(user_dir_str);

    while (getline(tokenStream, token, '/')) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }

    // Find the index of cur_user in the tokens
    size_t user_index = std::string::npos;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i] == cur_user) {
            user_index = i;
            break;
        }
    }

    char *prompt = (char *) malloc(sizeof(char) * 100);
    if (user_index == std::string::npos) {
        sprintf(prompt, "/%s", cur_user);
        return prompt;
    }

    // Construct the path from the user's directory onwards
    std::string path = "~/";
    for (size_t i = user_index + 1; i < tokens.size(); ++i) {
        path += tokens[i] + "/";
    }

    return path.c_str();
}