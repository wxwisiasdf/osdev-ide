#include <exception>
#include <iostream>
#include <memory>
#include "qemu.hpp"

std::string qemu_exec = "qemu-system-i386";

/// pipes a command and gets the stdout output of it
static std::string get_output(const char * cmd) {
	std::array<char,256> buffer;
	std::string result;
	std::unique_ptr<FILE,decltype(&pclose)> pipe(popen(cmd,"r"),pclose);
	if(!pipe) {
		throw std::runtime_error("popen failed\n");
	}
	while(fgets(buffer.data(),buffer.size(),pipe.get()) != nullptr) {
		result += buffer.data();
	}
	return result;
}

std::string qemu_get_traces(void) {
	std::string cmd = qemu_exec+" -trace help";
	return get_output(cmd.c_str());
}
