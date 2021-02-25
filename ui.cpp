#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Tabs.H>
#include <cstring>
#include "ui.hpp"
#include "project.hpp"
#include "qemu.hpp"

static unsigned fontsize = 24;

// Global styling
Fl_Text_Display::Style_Table_Entry styleTable[] = {
	{FL_BLACK,		FL_COURIER,			FL_NORMAL_SIZE}, // Plain
	{FL_BLACK,		FL_COURIER,			FL_NORMAL_SIZE}, // Plain
	{FL_RED,		FL_COURIER_BOLD,	FL_NORMAL_SIZE}  // Keywords
};

// File context vector
std::vector<std::tuple<Fl_Text_Editor *, Fl_Text_Buffer *, Fl_Text_Buffer *>> fileContext;

// File switcher
Fl_Window * window;
Fl_Tabs * fileTabs;
Fl_Tree * tree;
Fl_Menu_Bar * menuBar;

Fl_Window * traceSelWindow;
Fl_Tabs * traceSelTabs;

Fl_Tree * ui_project_to_tree(std::array<std::vector<std::string>, 2> project) {
	Fl_Tree * tree = new Fl_Tree(0,24,320,window->decorated_h()-fontsize);

	// Add directories to project tree, and show them with a nice folder icon
	for(const auto& dir: project.at(0)) {
		tree->add(dir.c_str());
	}

	// Add files
	for(const auto& file: project.at(1)) {
		tree->add(file.c_str());
	}

	tree->resizable(tree);
	tree->showroot(false);
	return tree;
}

std::regex ui_replace_dialog(Fl_Text_Buffer * buffer) {
	std::regex re;
	Fl_Window * dialog = new Fl_Window(300,75,"Replace");
	Fl_Input * find = new Fl_Input(100,0,200,24,"Find:");
	Fl_Input * replace = new Fl_Input(100,24,200,24,"Replace:");
	Fl_Button * replace_all = new Fl_Button(5,50,90,24,"Replace all");
	Fl_Button * replace_next = new Fl_Button(100,50,120,24,"Replace next");
	Fl_Button * cancel = new Fl_Button(225,50,60,24,"Cancel");

	dialog->add(find);
	dialog->add(replace);
	dialog->add(replace_all);
	dialog->add(replace_next);
	dialog->add(cancel);
	dialog->end();
	dialog->show();
	return re;
}

void ui_restyle(Fl_Text_Buffer * textBuffer, Fl_Text_Buffer * styleBuffer) {
	int sz = textBuffer->length();
	char * style = new char[sz+1];
	memset(style,'B',sz);
	style[sz] = '\0';
	styleBuffer->text(style);
	return;
}

std::tuple<Fl_Text_Editor *, Fl_Text_Buffer *, Fl_Text_Buffer *> ui_open_file(std::string filename, Fl_Tabs * tabs) {
	tabs->begin();

	size_t last_idx = filename.find_last_of('/');
	std::string disp_filename = filename;
	if(last_idx != 0) {
		disp_filename = &filename[last_idx+1];
	}

	char * cfile = new char[filename.length()+1];
	strcpy(cfile,filename.c_str());

	Fl_Group * fileTab = new Fl_Group(tree->w()+1,(fontsize*2),window->w()-tree->w()-2,window->h()-(fontsize*2));
	Fl_Text_Editor * editor = new Fl_Text_Editor(tree->w()+1,fontsize*2,window->w()-tree->w()-2,window->h()-(fontsize*2));
	Fl_Text_Buffer * text = new Fl_Text_Buffer();
	Fl_Text_Buffer * style = new Fl_Text_Buffer();

	text->insertfile(cfile,0);
	editor->buffer(text);
	editor->highlight_data(style,styleTable,sizeof(styleTable)/sizeof(styleTable[0]),'A',(Fl_Text_Display::Unfinished_Style_Cb)ui_restyle,0);
	ui_restyle(text,style);
	delete cfile;
	cfile = new char[disp_filename.length()+1];
	strcpy(cfile,disp_filename.c_str());
	fileTab->label(cfile);
	fileTab->end();

	tabs->end();
	tabs->value(fileTab);
	tabs->redraw();
	return std::make_tuple(editor,text,style);
}

// Creates new buffer when we click on the project tree
void ui_project_tree_callback(Fl_Widget * widget, void * data) {
	Fl_Tree * tree = (Fl_Tree *)widget;
	Fl_Tree_Item * item = tree->callback_item();
	if(item == nullptr) {
		return;
	}

	char * path = new char[8192];
	std::string end_path = "/";
	tree->item_pathname(path,8192,item);
	end_path += path;

	switch(tree->callback_reason()) {
	case FL_TREE_REASON_SELECTED:
		fileContext.push_back(ui_open_file(end_path,fileTabs));
		break;
	default:
		break;
	}
	return;
}

// TODO: Why it segfaults?
void ui_close_file(Fl_Widget * widget, void * data) {
	Fl_Group * group;
	group = fileTabs->current();
	return;
}

void ui_enable_traces_dialog(Fl_Widget * widget, void * data) {
	return;
}

#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
void ui_qemu_run(Fl_Widget * widget, void * data) {
	std::cout << "running qemu..." << std::endl;

	char buf[BUFSIZ];
	FILE * fp;
	fp = popen("qemu-system-i386 -trace scsi* -trace usb* -trace sata* -trace ata* -d unimp,guest_errors 2>&1","r");
	if(fp == NULL) {
		return;
	}
	while(fgets((char *)&buf,sizeof(buf),fp) != NULL) {
		std::cout << buf << std::endl;
	}

	std::cout << "pipe closed" << std::endl;

	pclose(fp);
	return;
}

int ui_main_loop(int argc, const char ** argv) {
	Fl_Menu_Item menuItems[] = {
		{"File",0,0,0,FL_SUBMENU},
			{"Create",FL_CTRL+'c',0,0},
			{"Open",FL_CTRL+'o',0,0},
			{"Close",FL_CTRL+'w',ui_close_file,0},
			{"Quit",0,0,0},
			{0},
		{"Edit",0,0,0,FL_SUBMENU},
			{"Undo",FL_CTRL+'z',0,0},
			{"Redo",FL_CTRL+'y',0,0},
			{"Find",FL_CTRL+'f',0,0},
			{"Replace",FL_CTRL+'r',0,0},
			{0},
		{"Options",0,0,0,0},
		{"Build",0,0,0,0},
		{"Run qemu",FL_CTRL+'q',ui_qemu_run,0,0},
		{0}
	};

	std::vector<Fl_Text_Buffer> buffers;
	window = new Fl_Window(1024,800,"OS DEVELOPEMENT ORIENTED IDE");

	// Make menu toolbar
	menuBar = new Fl_Menu_Bar(0,0,window->w(),fontsize);
	menuBar->menu(menuItems);
	window->add(menuBar);
	window->resizable(window);

	// Add project tree
	tree = ui_project_to_tree(project_get_paths("/home/superleaf1995/src/uDOS"));
	tree->callback(ui_project_tree_callback);
	window->add(tree);

	// Add file tabs
	fileTabs = new Fl_Tabs(tree->w()+1,24,window->w()-322,window->h()-fontsize);
	window->add(fileTabs);

	window->end();
	window->show(argc,(char **)argv);

	buffers.clear();
	// ui_replace_dialog(nullptr);
	return(Fl::run());
}
