#include <iostream>
#include <string>
#include <sstream>
#include "qemu.hpp"
#include "ui.hpp"

std::vector<std::string> trace_list;

int main(int argc, const char ** argv) {
	std::string trace;

	// Fill in list of traces
	trace = qemu_get_traces();

	std::istringstream iss(trace);
	std::string token;
	while(std::getline(iss,token,'\n')) {
		std::cout << trace << std::endl;
		trace_list.push_back(trace);
	}

	return ui_main_loop(argc,argv);
}
