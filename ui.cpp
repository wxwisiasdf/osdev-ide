#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Tabs.H>
#include <ctype.h>
#include <cstring>
#include "ui.hpp"
#include "project.hpp"
#include "qemu.hpp"

static unsigned fontsize = 24;

// Global styling
Fl_Text_Display::Style_Table_Entry styleTable[] = {
	{FL_BLACK,		FL_COURIER,			FL_NORMAL_SIZE}, // Plain
	{FL_DARK_GREEN,	FL_COURIER,			FL_NORMAL_SIZE}, // Macros
	{FL_GRAY,		FL_COURIER_ITALIC,	FL_NORMAL_SIZE}, // Comments
	{FL_BLUE,		FL_COURIER,			FL_NORMAL_SIZE}, // Number
	{FL_BLUE,		FL_COURIER_BOLD,	FL_NORMAL_SIZE}, // Keywords
	{FL_RED,		FL_COURIER,			FL_NORMAL_SIZE}, // Strings
};

// File context vector
std::vector<std::tuple<Fl_Text_Editor *, Fl_Text_Buffer *, Fl_Text_Buffer *>> fileContext;
std::vector<std::tuple<std::string, Fl_Text_Editor *>> fullFileName;

// File switcher
Fl_Window * window;
Fl_Tabs * fileTabs;
Fl_Tree * tree;
Fl_Menu_Bar * menuBar;

Fl_Window * traceSelWindow;
Fl_Tabs * traceSelTabs;

Fl_Text_Editor * ui_get_current_text_editor(void) {
	Fl_Widget * tabOfFile;
	tabOfFile = fileTabs->value();
	if(tabOfFile == NULL) {
		printf("No file to close\n");
		return NULL;
	}

	Fl_Group * tabOfFileGroup;
	tabOfFileGroup = (Fl_Group *)tabOfFile;
	if(tabOfFileGroup == NULL) {
		printf("No tab file group within tab\n");
		return NULL;
	}

	if(tabOfFileGroup->children() != 0) {
		Fl_Widget * const * fileWidget;
		fileWidget = tabOfFileGroup->array();
		if(fileWidget == NULL) {
			printf("Group contains no widgets\n");
			return NULL;
		}

		for(int i = 0; i < tabOfFileGroup->children(); i++) {
			Fl_Widget * expectedFileWidgetInFileGroupTab;
			expectedFileWidgetInFileGroupTab = fileWidget[i];

			Fl_Text_Editor * fileEditor = (Fl_Text_Editor *)expectedFileWidgetInFileGroupTab;
			return fileEditor;
		}
	}
	return NULL;
}

Fl_Tree * ui_project_to_tree(std::array<std::vector<std::string>, 2> project) {
	//tree = new Fl_Tree(0,24,320,window->decorated_h()-fontsize);

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

const char * keywords[] = {
	"template","NULL","nullptr","static","inline","const","char","unsigned",
	"size_t","void","halfptr","dynamic_cast","static_cast","case","switch",
	"if","else","goto","break","continue","while","do","for","uint8_t",
	"uint16_t","uint32_t","uint64_t","uint128_t","shared_ptr","class",
	"new","delete","return","auto","ssize_t","ptrdiff_t","pthread_t",
	"extern","int","long","typedef","typeof","sizeof","NULLPTR",
	"struct","union","enum","FILE","interrupt","_Cdecl","asm","__asm__",
	"__volatile__","volatile","namespace","using","true","false","fullptr",
	"exception","bool","throw","catch","public","private","virtual","uintptr_t",
	"intptr_t","int16_t","int8_t","int32_t","int64_t","int128_t","float","double",
	"float_t"
};

void ui_restyle(Fl_Text_Buffer * textBuffer, Fl_Text_Buffer * styleBuffer) {
	int sz = textBuffer->length();
	char * style = new char[sz+1];

	// Paint everything of color black
	memset(style,'A',sz);
	style[sz] = '\0';

	// Paint depending on keywords
	const char * text = textBuffer->text();
	for(size_t i = 0; i < sz; i++) {
		const char * end_of;
		size_t diff;
		int base; // used for strings only

		if(text[i] == '#') {
			printf("macro\n");
			end_of = strchr(&text[i],'\n');
			if(end_of == NULL) {
				continue;
			}

			diff = strlen(&text[i])-strlen(end_of);
			memset(&style[i],'B',diff);
			i += diff;
		} else if(text[i] == '/') {
			bool is_multi;
			end_of = NULL;
			if(text[i+1] == '/') { // single line comment
				end_of = strchr(&text[i],'\n');
				is_multi = false;
			} else if(text[i+1] == '*') { // multi line comment
				end_of = strstr(&text[i],"*/");
				is_multi = true;
			}
			if(end_of == NULL) {
				continue;
			}
			diff = strlen(&text[i])-strlen(end_of);
			if(is_multi) {
				diff += 2;
			}
			memset(&style[i],'C',diff);
			i += diff;
		} else if(text[i] == '\'' || text[i] == '"') {
			// strings require a bit of special parsing
			base = text[i];
			style[i] = 'F';
			i++;
			while(text[i] != base) {
				style[i] = 'F';
				if(text[i] == '\\') {
					style[i+1] = 'F';
					i += 2;
					continue;
				}
				i++;
			}
			style[i] = 'F';
		} else if(text[i] >= '0' && text[i] <= '9') {
			printf("number\n");
			// a number
			while(isalnum(text[i])) {
				style[i] = 'D';
				i++;
			}
		} else {
			printf("keyword\n");
			// check keywording
			const char * wspace = strpbrk(&text[i]," \t\n()*&[],\"'.-<>=!#%^;:");
			if(wspace == NULL) {
				continue;
			}
			diff = strlen(&text[i])-strlen(wspace);
			if(!diff) {
				continue;
			}
			char * partText = new char[diff+1];
			strncpy(partText,&text[i],diff); // strcpy is just sugar memcpy
			partText[diff] = '\0';
			for(size_t j = 0; j < (sizeof(keywords)/sizeof(const char *))-1; j++) {
				if(strcmp(keywords[j],partText)) {
					continue;
				}

				memset(&style[i],'E',strlen(keywords[j]));
				i += diff;
				break;
			}
			delete partText;
		}
	}

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

	Fl_Group * tabOfFile = new Fl_Group(tree->w()+1,(fontsize*2),window->w()-tree->w()-2,window->h()-(fontsize*2));
	Fl_Text_Editor * editor = new Fl_Text_Editor(tree->w()+1,fontsize*2,window->w()-tree->w()-2,window->h()-(fontsize*2));
	Fl_Text_Buffer * text = new Fl_Text_Buffer();
	Fl_Text_Buffer * style = new Fl_Text_Buffer();

	fullFileName.push_back(std::make_tuple(cfile,editor));

	text->insertfile(cfile,0);
	ui_restyle(text,style);

	editor->buffer(text);
	editor->highlight_data(style,styleTable,sizeof(styleTable)/sizeof(styleTable[0]),'A'-1,(Fl_Text_Display::Unfinished_Style_Cb)ui_restyle,0);
	delete cfile;
	cfile = new char[disp_filename.length()+1];
	strcpy(cfile,disp_filename.c_str());
	tabOfFile->label(cfile);
	tabOfFile->end();

	tabs->end();
	tabs->value(tabOfFile);
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

void ui_close_file(Fl_Widget * widget, void * data) {
	Fl_Widget * tabOfFile;
	tabOfFile = fileTabs->value();
	if(tabOfFile == NULL) {
		printf("No file to close\n");
		return;
	}
	fileTabs->remove(tabOfFile);
	delete tabOfFile;

	// We need to redraw everything
	window->redraw();
	return;
}

void ui_save_file(Fl_Widget * widget, void * data) {
	Fl_Text_Editor * editor;
	editor = ui_get_current_text_editor();

	Fl_Text_Buffer * fileBuffer = editor->buffer();
	if(fileBuffer == NULL)
		return;

	// Find in array
	for(auto& o: fullFileName) {
		if(std::get<Fl_Text_Editor *>(o) != editor) {
			continue;
		}
		std::string filename;
		filename = std::get<std::string>(o);
			FILE * fp;
		fp = fopen(filename.c_str(),"wt");
		if(fp == NULL) {
			printf("Cannot truncate file\n");
			return;
		}
		fputs(fileBuffer->text(),fp);
		fclose(fp);
	}
	return;
}

void ui_enable_traces_dialog(Fl_Widget * widget, void * data) {
	return;
}

#include <unistd.h>
void ui_qemu_run(Fl_Widget * widget, void * data) {
	printf("running qemu...\n");

	char buf[BUFSIZ];
	FILE * fp;
	fp = popen("qemu-system-i386 -trace pci* -trace scsi* -trace usb* -trace sata* -trace ata* -d unimp,guest_errors","r");
	if(fp == NULL) {
		return;
	}

	Fl_Text_Buffer * txtBuf = new Fl_Text_Buffer();
	while(fgets((char *)&buf,sizeof(buf),fp) != NULL) {
		txtBuf->append(buf);
		//std::cout << buf << std::endl;
	}

	printf("%s\n",txtBuf->text());

	std::tuple<Fl_Text_Editor *, Fl_Text_Buffer *, Fl_Text_Buffer *> logOutput;
	logOutput = ui_open_file("/tmp/logoutput_osdev_ide.txt",fileTabs);
	std::get<1>(logOutput) = txtBuf;
	fileContext.push_back(logOutput);

	printf("pipe closed\n");

	pclose(fp);
	return;
}

void ui_when_project_gets_opened(Fl_File_Chooser * widget, void * data) {
	const char * dir = widget->directory();
	tree = ui_project_to_tree(project_get_paths(dir));
	return;
}

void ui_open_project(Fl_Widget * widget, void * data) {
	//const char * pattern = "C Files (*.c)\tC Header Files (*.h)\tC++ Source Files (*.cpp)\tASM Source code (*.S)";

	const char * dir;
	dir = fl_dir_chooser("Open directory","/home",0);
	printf("Directory opened: %s\n",dir);
	if(dir != NULL) {
		tree = ui_project_to_tree(project_get_paths(dir));
	}
	return;
}

int ui_main_loop(int argc, const char ** argv) {
	Fl_Menu_Item menuItems[] = {
		{"File",0,0,0,FL_SUBMENU},
			{"Open project",FL_CTRL+'p',ui_open_project,0},
			{"Create",FL_CTRL+'c',0,0},
			{"Open",FL_CTRL+'o',0,0},
			{"Save",FL_CTRL+'s',ui_save_file,0},
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
	window = new Fl_Window(1024,800,"Operating System Developement Oriented IDE");

	// Make menu toolbar
	menuBar = new Fl_Menu_Bar(0,0,window->w(),fontsize);
	menuBar->menu(menuItems);
	window->add(menuBar);
	window->resizable(window);

	// Add project tree
	//tree = ui_project_to_tree(project_get_paths("/home/superleaf1995/src/uDOS"));

	// Create empty project tree
	tree = new Fl_Tree(0,24,320,window->decorated_h()-fontsize);
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
