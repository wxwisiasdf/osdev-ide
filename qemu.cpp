#include <iostream>
#include <clocale>
#include <cstdio>
#include <exception>
#include <memory>
#include "qemu.hpp"
#include "ui.hpp"
#include "tab.hpp"

std::vector<QemuDevice *> globalQemuDeviceList;
std::string qemu_cmd = "qemu-system-x86_64";
std::string qemu_args = "-d int,unimp,guest_errors -trace usb* -trace nvme* -trace virtio* -trace pci* -trace net*";

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
	std::string cmd = qemu_cmd+" -trace help";
	return get_output(cmd.c_str());
}

#include <sstream>
#include <cstring>
#include <FL/Fl_Button.H>

// qemu create image
Fl_Window * qci_dialog;
Fl_Button * qci_cancel, * qci_create;

void qemu_create_image_dialog(void) {
	// Create replaceace dialog
	qci_dialog = new Fl_Window(640,400,"Create new disk image");
	qci_dialog->begin();
	qci_cancel = new Fl_Button(8,200+globalFontsize*6,64,globalFontsize,"Cancel");
	qci_create = new Fl_Button(96,200+globalFontsize*6,64,globalFontsize,"Create");
	qci_dialog->end();
	return;
}

#include <cstdlib>
#include <unistd.h>
#include "tab.hpp"
#include "editor.hpp"
#include "project.hpp"
void qemu_run_callback(Fl_Widget * widget, void * data) {
	// new temporal files
	auto buf = std::unique_ptr<char[]>(new char[BUFSIZ]);

	// create commands
	const char * qlog = tempnam("/tmp","ql");
	tab_open_file(qlog,globalTabManager);

	// change directory
	chdir(globalCurrentDir.c_str());

	// execute in this order
	//execl(qemu_cmd.c_str(),redirect_args.c_str());
	std::string joint = qemu_cmd+" "+qemu_args+" 2>&1";
	printf("running \'%s\'",joint.c_str());
	std::unique_ptr<FILE,decltype(&pclose)> pfp(popen(joint.c_str(),"r"),pclose);
	Fl_Text_Editor * editor = editor_current();
	if(editor == NULL) {
		return;
	}
	Fl_Text_Buffer * textbuf = editor->buffer();
	textbuf->text("QEMU Log output:\n");
	textbuf->append(qemu_cmd.c_str());
	textbuf->append(" ");
	textbuf->append(qemu_args.c_str());
	textbuf->append("\n\n");
	while(fgets(buf.get(),BUFSIZ,pfp.get()) != NULL) {
		textbuf->append(buf.get());
		Fl::wait(); // TODO: Perform one tick, this might not be the best way to handle pipes
	}
}

#include "device_man.hpp"
std::vector<QemuDevice *> qemu_get_devices(void) {
	std::string cmd = qemu_cmd+" -device help";
	std::string text = get_output(cmd.c_str());
	std::istringstream textstream(text);
	std::vector<QemuDevice *> dev;
	std::string line;
	auto nname = std::unique_ptr<char[]>(new char[255]);
	while(std::getline(textstream,line)) {
		std::sscanf(line.c_str(),"%*s %255s",nname.get());

		// Name starts at quote
		char * name = strchr(nname.get(),'"');
		if(name == NULL) {
			continue;
		}
		name++;

		// Remove enclosing quote
		char * enclosing = strchr(name,'"');
		if(enclosing != NULL) {
			enclosing[0] = '\0';
		}

		QemuDevice * ndev = new QemuDevice();
		printf("Name: %s\n",name);

		ndev->name = new char[strlen(name)];
		ndev->count = 0;
		ndev->name = name;
		dev.push_back(ndev);

		std::string endItem = "Devices/"+std::string(name);
		globalMenuBar->insert(-1,endItem.c_str(),0,qemu_add_device_callback,(void *)ndev->name.c_str(),0);
	}
	return dev;
}
