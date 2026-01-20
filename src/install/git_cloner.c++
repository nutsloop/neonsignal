#include "install/git_cloner.h++"

#include <string>
#include <utility>

namespace neonsignal::install {

GitCloner::GitCloner(std::string repo_url, std::optional<std::string> branch)
    : repo_url_(std::move(repo_url)), branch_(std::move(branch)) {
}

} // namespace neonsignal::install
