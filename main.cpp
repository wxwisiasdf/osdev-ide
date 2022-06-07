#include <iostream>
#include <string>
#include <sstream>
#include "qemu.hpp"
#include "ui.hpp"

#if 0
std::vector<std::string> trace_list;
#endif

int main(int argc, const char ** argv) {
	std::string trace;

#if 0
	// Fill in list of traces
	trace = qemu_get_traces();
	std::istringstream iss(trace);
	std::string token;
	while(std::getline(iss,token,'\n')) {
		std::cout << trace << std::endl;
		trace_list.push_back(trace);
	}
#endif

	return ui_main_loop(argc,argv);
}
