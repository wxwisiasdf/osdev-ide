#ifndef QEMU_HPP_INCLUDED
#define QEMU_HPP_INCLUDED

#include <string>
#include <vector>
#include <FL/Fl.H>

class QemuDevice {
public:
	std::string name;
	int count;
};

extern std::vector<QemuDevice *> globalQemuDeviceList;
extern std::string qemu_cmd;
extern std::string qemu_args;

extern std::string qemu_exec;
std::string qemu_get_traces(void);
void qemu_run_callback(Fl_Widget * widget, void * data);
std::vector<QemuDevice *> qemu_get_devices(void);

#endif // QEMU_HPP_INCLUDED
