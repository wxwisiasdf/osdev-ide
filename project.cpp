#include <version>
#include <array>
#include <experimental/array>
#ifdef __cpp_lib_filesystem
#	include <filesystem>
#elif __cpp_lib_experimental_filesystem
#	include <experimental/filesystem>
namespace std {
	namespace filesystem = experimental::filesystem;
};
#endif
#include "project.hpp"

// File context vector
std::string globalCurrentDir;
Fl_Tree * globalProjectTree;

// Recursive fucntion
static inline std::array<std::vector<std::string>, 2> project_recurse(std::string root) {
	std::vector<std::string> files, dirs;
	for(const auto& current: std::filesystem::directory_iterator(root)) {
		// Recurse onto directory
		if(std::filesystem::is_directory(current.path())) {
			std::array<std::vector<std::string>, 2> temp;
			temp = project_recurse(current.path());
			for(const auto& item: std::get<0>(temp)) {
				files.push_back(item);
			}
			for(const auto& item: std::get<1>(temp)) {
				dirs.push_back(item);
			}
		}
		if(std::filesystem::is_directory(current.path())) {
			dirs.push_back(current.path());
		} else {
			files.push_back(current.path());
		}
	}
	return std::experimental::make_array(files,dirs);
}

// Exposed prototype for recursive function wrapping
std::array<std::vector<std::string>, 2> project_get_paths(std::string root) {
	return project_recurse(root);
}
