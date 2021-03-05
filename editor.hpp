#ifndef EDITOR_HPP_INCLUDED
#define EDITOR_HPP_INCLUDED

#include <FL/Fl.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>

enum {
	EDITOR_CHANGE_DELETE,
	EDITOR_CHANGE_INSERT
};

class EditorChange {
	int type;
	int pos;
	const char * text;
};

Fl_Text_Editor * editor_current(void);
void editor_undo(Fl_Widget * widget, void * data);
void editor_redo(Fl_Widget * widget, void * data);
void editor_copy(Fl_Widget * widget, void * data);
void editor_paste(Fl_Widget * widget, void * data);
void editor_cut(Fl_Widget * widget, void * data);
void editor_replace_next_callback(Fl_Widget * widget, void * data);
void editor_replace_all_callback(Fl_Widget * widget, void * data);
void editor_find_callback(Fl_Widget * widget, void * data);

#endif // EDITOR_HPP_INCLUDED
