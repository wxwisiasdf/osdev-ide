#include <FL/fl_ask.H>
#include "editor.hpp"
#include "tab.hpp"

Fl_Text_Editor * editor_current(void) {
	FileTab * tab;

	tab = tab_current();
	Fl_Group * tabGroup = tab->tabref;

	if(tabGroup->children() != 0) {
		Fl_Widget * const * fileWidget;
		fileWidget = tabGroup->array();
		if(fileWidget == NULL) {
			return NULL;
		}

		for(int i = 0; i < tabGroup->children(); i++) {
			Fl_Widget * expectedFileWidgetInFileGroupTab;
			expectedFileWidgetInFileGroupTab = fileWidget[i];

			Fl_Text_Editor * fileEditor = (Fl_Text_Editor *)expectedFileWidgetInFileGroupTab;
			return fileEditor;
		}
	}
	return NULL;
}

void editor_undo(Fl_Widget * widget, void * data) {
	Fl_Text_Editor * editor = editor_current();
	Fl_Text_Editor::kf_undo(0,editor);
}

void editor_redo(Fl_Widget * widget, void * data) {
	Fl_Text_Editor * editor = editor_current();
	// TODO: Implement
}

void editor_copy(Fl_Widget * widget, void * data) {
	Fl_Text_Editor * editor = editor_current();
	Fl_Text_Editor::kf_copy(0,editor);
}

void editor_paste(Fl_Widget * widget, void * data) {
	Fl_Text_Editor * editor = editor_current();
	Fl_Text_Editor::kf_paste(0,editor);
}

void editor_cut(Fl_Widget * widget, void * data) {
	Fl_Text_Editor * editor = editor_current();
	Fl_Text_Editor::kf_cut(0,editor);
}

#include "find.hpp"
void editor_replace_next_callback(Fl_Widget * widget, void * data) {
	Fl_Text_Editor * editor = editor_current();
	Fl_Text_Buffer * textbuf = editor->buffer();
	find_replace_next(editor,textbuf);
}

void editor_replace_all_callback(Fl_Widget * widget, void * data) {
	Fl_Text_Editor * editor = editor_current();
	Fl_Text_Buffer * textbuf = editor->buffer();
	find_replace_all(editor,textbuf);
}

void editor_find_callback(Fl_Widget * widget, void * data) {
	Fl_Text_Editor * editor = editor_current();
	Fl_Text_Buffer * textbuf = editor->buffer();

	const char * tfind = fl_input("Find:");
	if(!strlen(tfind)) {
		return;
	}
	int pos = editor->insert_position();
	int found = textbuf->search_forward(pos,tfind,&pos);
	if(found) {
		editor->insert_position(pos);
		editor->show_insert_position();
		textbuf->select(pos,pos+strlen(tfind));
	} else {
		fl_alert("No occurrences found\n");
	}
	return;
}
