#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Group.H>
#include <memory>
#include "tab.hpp"
#include "buffer.hpp"
#include "project.hpp"
#include "ui.hpp"

Fl_Tabs * globalTabManager;
std::vector<FileTab *> globalTabs;

FileTab * tab_current(void) {
	Fl_Group * currentTab = (Fl_Group *)globalTabManager->value();
	for(const auto& file: globalTabs) {
		if(file->tabref != currentTab) {
			continue;
		}
		return file;
	}
	return NULL;
}

static int recur_lock = 0;
void tab_on_edit(int pos, int nInserted, int nDeleted, int nRestyled, const char * delText, void * cbArg) {
	Fl_Text_Buffer * stylebuf, * textbuf;
	FileTab * tab;

	stylebuf = (Fl_Text_Buffer *)cbArg;

	tab = tab_current();
	if(tab == NULL) {
		return;
	}

	textbuf = tab->editor->buffer();
	if(textbuf == NULL) {
		return;
	}

	if(nInserted == 0 && nDeleted == 0) {
		stylebuf->unselect();
		return;
	}

	// Auto-brace completion
	const char * all_text = textbuf->text();
	// TODO: This calls itself recursively
	if(!recur_lock && nDeleted == 0 && pos > 0 && all_text[pos] == '\n') { // Handle braces
		int i = pos-1;
		// Go to the previous newline
		while(i != 0) {
			if(all_text[i] == '\n') {
				i++;
				break;
			}
			i--;
		}
		// Analyze the tabs
		std::string identStr = "";
		while(all_text[i] == '\t') {
			identStr += "\t";
			i++;
		}
		// Add an extra level of identation on braces
		if(pos-1 >= 0 && all_text[pos-1] == '{') {
			identStr += "\t";
		}
		
		recur_lock = 1;
		textbuf->insert(pos+1,identStr.c_str());
		recur_lock = 0;

		for(const auto& ch : identStr) {
			if(ch == '\t' || ch == ' ') {
				tab->editor->move_right();
			} else if(ch == '\n') {
				tab->editor->move_down();
			}
		}
	}

	// reparse entire line now
	int start = textbuf->line_start(pos);
	int end = textbuf->line_end(pos);

    // Insert characters into the style buffer...
	if(nInserted > 0) {
		auto style = std::unique_ptr<char[]>(new char[nInserted+1]);
		memset(style.get(),'A',nInserted);
		style[nInserted] = '\0';
		stylebuf->replace(pos,pos+nDeleted,style.get());
	} else {
		// Remove from the style buffer
		stylebuf->remove(pos,pos+nDeleted);
	}

	const char * text = textbuf->text_range(start,end);
	char * new_style = stylebuf->text_range(start,end);
	buffer_syntax_hl(textbuf,stylebuf,start,end,new_style);
	stylebuf->replace(start,end,new_style);
	return;
}

#include "syntax/asm.hpp"
#include "syntax/c.hpp"
#include "syntax/rust.hpp"
#include "editor.hpp"
#include <unistd.h>
FileTab * tab_open_file(std::string filename, Fl_Tabs *) {
	FileTab * newFileTab = new FileTab();
	globalTabManager->begin();

	Fl_Group * fileTab = new Fl_Group(globalProjectTree->w()+1,(globalFontsize*2),globalWindow->w()-globalProjectTree->w()-2,globalWindow->h()-(globalFontsize*2));
	Fl_Text_Editor * editor = new Fl_Text_Editor(globalProjectTree->w()+1,globalFontsize*2,globalWindow->w()-globalProjectTree->w()-2,globalWindow->h()-(globalFontsize*2));
	Fl_Text_Buffer * textbuf = new Fl_Text_Buffer();
	Fl_Text_Buffer * style = new Fl_Text_Buffer();

	textbuf->text("");

	size_t last = filename.find_last_of('.');
	const char * ext = filename.c_str();
	newFileTab->parser = NULL;
	newFileTab->type = FILETAB_SOURCE_CODE;
	if(ext != NULL) {
		ext = &ext[last+1];

		// source code stuff
		if(!strcasecmp("c",ext) || !strcasecmp("cpp",ext) || !strcasecmp("cxx",ext) || !strcasecmp("hpp",ext) || !strcasecmp("h",ext)) {
			newFileTab->parser = &c_parser;
			newFileTab->type = FILETAB_SOURCE_CODE;
		} else if(!strcasecmp("rust",ext) || !strcasecmp("rs",ext)) {
			newFileTab->parser = &rust_parser;
			newFileTab->type = FILETAB_SOURCE_CODE;
		} else if(!strcasecmp("asm",ext) || !strcasecmp("s",ext)) {
			newFileTab->parser = &asm_parser;
			newFileTab->type = FILETAB_SOURCE_CODE;
		}
		// special handling for elf files, objdump them automagically
		else if(!strcasecmp("o",ext) || !strcasecmp("elf",ext) || !strcasecmp("a",ext)) {
			newFileTab->type = FILETAB_EXEC_NAVIGATOR;
			FILE * fp;
			chdir(globalCurrentDir.c_str());
			std::string cmd = "objdump -t -T -d ";
			cmd += filename;
			fp = popen(cmd.c_str(),"r");
			if(fp == NULL) {
				return NULL;
			}
			auto buf = std::unique_ptr<char[]>(new char[BUFSIZ]);
			while(fgets(buf.get(),BUFSIZ,fp) != NULL) {
				textbuf->append(buf.get());
				Fl::wait(); // TODO: Perform one tick, this might not be the best way to handle pipes
			}
			pclose(fp);
			newFileTab->parser = &asm_parser;
		}
	}

	// Full path pointer (given to widget FileTab object later)
	auto fullpath = std::unique_ptr<char[]>(new char[filename.length()+1]);
	strcpy(fullpath.get(),filename.c_str());

	// Insert file onto buffer
	switch(newFileTab->type) {
	case FILETAB_SOURCE_CODE:
		textbuf->insertfile(fullpath.get(),0);
		break;
	case FILETAB_EXEC_NAVIGATOR:
		break;
	default:
		break;
	}

	textbuf->add_modify_callback(tab_on_edit,style);
	editor->buffer(textbuf);
	editor->highlight_data(style,globalStyleTable,sizeof(globalStyleTable)/sizeof(globalStyleTable[0]),'A'-1,(Fl_Text_Display::Unfinished_Style_Cb)buffer_restyle,0);

	// Add line numbers
	editor->linenumber_width(48);
	editor->linenumber_format("%04d");

	// Pretty name
	size_t last_idx = filename.find_last_of('/');
	char * displayFilename = new char[filename.length()];
	if(last_idx != 0) {
		strcpy(displayFilename,&filename[last_idx+1]);
	} else {
		strcpy(displayFilename,filename.c_str());
	}

	// Add label (with file name + extension) so it looks pretty
	fileTab->label(displayFilename);
	fileTab->end();

	// Add tab to the tab group, and assign it as the currently active tab
	globalTabManager->end();
	globalTabManager->value(fileTab);
	globalTabManager->redraw();

	// Construct object
	newFileTab->textbuf = textbuf;
	newFileTab->stylebuf = style;
	newFileTab->editor = editor;
	newFileTab->full_name = fullpath.get();
	newFileTab->tabref = fileTab;

	// Add to global list
	globalTabs.push_back(newFileTab);

	// Do a quick restyle on the entire file
	buffer_restyle(newFileTab->textbuf,newFileTab->stylebuf);
	return newFileTab;
}
