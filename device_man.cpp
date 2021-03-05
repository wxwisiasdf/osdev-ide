#include <FL/Fl_Input.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Button.H>
#include "qemu.hpp"
#include "ui.hpp"

// qemu create device
Fl_Window * qcd_dialog;
Fl_Input * qcd_id, * qcd_pcibus, * qcd_chardev, * qcd_extra, * qcd_name;
Fl_Help_View * qcd_help;
Fl_Button * qcd_cancel, * qcd_create;

static const char * html_qcd_help = {
"<html>"
"	<body>"
"		<h1>Adding QEMU devices</h1>"
"		<p><b>Add device</b> - Name of device to add</p>"
"		<p><b>Identifier</b> - ID assigned when referencing this device in QEMU</p>"
"		<p><b>Chardev</b> - Specifies the character device this device will output to</p>"
"		<p><b>PCI Bus</b> - Specifies the PCI bus for this device, leave blank if you do not manually manage PCI devices</p>"
"		<p><b>Extra parameters</b> - A comma separated list (i.e: 'mem_mb=4,foo=bar,der=not') of properties specific to a device</p>"
"	</body>"
"</html>"
};

static void qemu_hide_device_callback(Fl_Widget * widget, void * data) {
	qcd_dialog->hide();
	return;
}

static void qemu_add_device_to_arguments_callback(Fl_Widget * widget, void * data) {
	std::string tmp = qcd_name->value();
	if(tmp.length()) {
		std::string new_dev = "";
		new_dev += " -device "+tmp;

		tmp = qcd_id->value();
		if(tmp.length()) {
			new_dev += ",id="+tmp;
		}

		tmp = qcd_pcibus->value();
		if(tmp.length()) {
			new_dev += ",bus="+tmp;
		}

		tmp = qcd_chardev->value();
		if(tmp.length()) {
			new_dev += ",chardev="+tmp;
		}

		tmp = qcd_extra->value();
		if(tmp.length()) {
			new_dev += ","+tmp;
		}

		printf("Ouput command: %s\n",new_dev.c_str());
		qemu_args += " "+new_dev;
	}
	qcd_dialog->hide();
	return;
}

void qemu_add_device_callback(Fl_Widget * widget, void * data) {
	const char * name = (const char *)data;
	qcd_name->value(name);
	qcd_dialog->show();
	return;
}

void qemu_create_device_dialog(void) {
	// Create replaceace dialog
	qcd_dialog = new Fl_Window(640,400,"Add new device");
	qcd_dialog->begin();

	qcd_name = new Fl_Input(150,0,300,globalFontsize,"Name:");
	qcd_id = new Fl_Input(150,globalFontsize,300,globalFontsize,"Identifier:");
	qcd_pcibus = new Fl_Input(150,globalFontsize*2,300,globalFontsize,"PCI bus:");
	qcd_chardev = new Fl_Input(150,globalFontsize*3,300,globalFontsize,"Chardev:");
	qcd_extra = new Fl_Input(150,globalFontsize*4,300,globalFontsize,"Extra parameters:");
	qcd_help = new Fl_Help_View(0,globalFontsize*6,635,200);
	qcd_help->value(html_qcd_help);

	qcd_cancel = new Fl_Button(8,200+globalFontsize*6,64,globalFontsize,"Cancel");
	qcd_create = new Fl_Button(96,200+globalFontsize*6,64,globalFontsize,"Create");

	qcd_cancel->callback(qemu_hide_device_callback);
	qcd_create->callback(qemu_add_device_to_arguments_callback);

	qcd_dialog->end();
	return;
}
