#include <version>
#include <array>
#include <memory>
#include <experimental/array>
#include <iostream>
#include <filesystem>
#include <thread>
#include "project.hpp"

// File context vector
std::string globalCurrentDir;
Fl_Tree * globalProjectTree;
Project globalProject;

// Recursive fucntion
std::array<std::vector<std::string>, 2> Project::recurse_all(const std::string& root) {
	std::vector<std::string> files, dirs;
	for(const auto& current: std::filesystem::directory_iterator(root)) {
		// Recurse onto directory
		if(std::filesystem::is_directory(current.path())) {
			std::array<std::vector<std::string>, 2> temp;
			temp = this->recurse_all(current.path());
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

void Project::parse_make(const std::string& filename) {
	FILE * fp = fopen(filename.c_str(),"r");

	std::vector<std::string> rules, phonys;

	auto buf = std::unique_ptr<char[]>(new char[BUFSIZ]);
	while(fgets(buf.get(),BUFSIZ,fp)) {
		size_t i = 0;
		if(buf[i] == '.') {
			i++;
			if(!strncmp(&buf[i],"PHONY",5)) {
				char *ptr = NULL;
				i += 6; // Skip "PHONY ", notice the space

				char * token = strtok_r(&buf[i], " ", &ptr);
				while(token != NULL) {
					// Remove newlines
					size_t nl_idx = std::string(token).find_last_of('\n');
					if(nl_idx != std::string::npos) {
						token[nl_idx] = '\0';
					}

					std::cout << "Make PHONY " << token << std::endl;
					phonys.push_back(token);
					token = strtok_r(NULL, " ", &ptr);
				}
				continue; // Go to next line
			}
		}

		while(isalnum(buf[i])) {
			i++;
		}
		// We want to find semicolons as those are to dictate make rules
		if(buf[i] == ':') {
			const std::string rule_name = std::string(buf.get()).substr(0,i);
			rules.push_back(rule_name); // Single-threaded rule
		}
	}
	fclose(fp);

	// Now obtain PHONY's
	for(const auto& phony : phonys) {
		this->buildRules["Make (" + phony + ")"] = "make "+phony+" -j"+std::to_string(std::thread::hardware_concurrency());
	}
}

void project_open(const std::string& root) {
	Project * project = &globalProject;
	project->paths = project->recurse_all(root);

	// Detect build rules
	for(const auto& full_path: project->paths.at(0)) {
		std::string file = std::filesystem::path(full_path).filename();
		std::cout << "*" << file << std::endl;

		// Check extension of file
		size_t last = file.find_last_of('.');
		const char * ext = file.c_str();
		if(ext != NULL) {
			ext = &ext[last+1];
			// Shell build scripts, configure?
			if(!strcasecmp("sh",ext)) {
				if(!strcasecmp("build.sh",file.c_str())) {
					project->buildRules["Build"] = "chmod +x build.sh && sh build.sh";
				} else if(!strcasecmp("configure.sh",file.c_str())) {
					project->buildRules["Configure"] = "chmod +x build.sh && sh configure.sh";
				}
			} else if(!strcasecmp("ac",ext)) {
				if(!strcasecmp("configure.ac",file.c_str())) {
					project->buildRules["Autoconf configure"] = "chmod +x build.sh && automake --add-missing && autoconf";
				}
			}
		}

		if(!strcasecmp("Makefile",file.c_str())) {
			project->buildRules["Make (parallel)"] = "make -j" + std::to_string(std::thread::hardware_concurrency());
			project->buildRules["Make"] = "make";
			project->parse_make(full_path);
		} else if(!strcasecmp("GNUmakefile",file.c_str())) {
			project->buildRules["GNU Make (parallel)"] = "make -f "+file+" -j"+std::to_string(std::thread::hardware_concurrency());
			project->buildRules["GNU Make"] = "make -f"+file;
			project->parse_make(full_path);
		}
	}
}
