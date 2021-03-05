#ifndef TAB_HPP_INCLUDED
#define TAB_HPP_INCLUDED

#include <vector>
#include <string>
#include <FL/Fl.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Tabs.H>
#include <stack>
#include "editor.hpp"

enum {
	FILETAB_EXEC_NAVIGATOR,
	FILETAB_SOURCE_CODE
};

class FileTab {
public:
	Fl_Text_Buffer * textbuf;		// text buffer
	Fl_Text_Buffer * stylebuf;		// style buffer
	Fl_Text_Editor * editor;		// editor widget
	Fl_Group * tabref;				// group referencing the tab
	std::string full_name;			// full name of file (absolute path)
	void (*parser)(const char * text, char * style, int length);
	int type;
	std::stack<EditorChange> changes;
};

extern std::vector<FileTab *> globalTabs;
extern Fl_Tabs * globalTabManager;

FileTab * tab_current(void);
void tab_on_edit(int pos, int nInserted, int nDeleted, int nRestyled, const char * delText, void * cbArg);
FileTab * tab_open_file(std::string filename, Fl_Tabs *);
Fl_Text_Editor * tab_get_text_editor(void);

#endif // TAB_HPP_INCLUDED
