#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>
#include <cstring>
#include "find.hpp"
#include "ui.hpp"
#include "editor.hpp"

Fl_Window * replace_dialog;
Fl_Input * replace_find;
Fl_Input * replace_replace;
Fl_Button * replace_replace_all;
Fl_Button * replace_replace_next;
Fl_Button * replace_cancel;

void find_replace_next(Fl_Text_Editor * editor, Fl_Text_Buffer * textbuf) {
	const char * tfind = replace_find->value();
	const char * treplace = replace_replace->value();
	if(!strlen(tfind)) {
		replace_dialog->show();
		return;
	}

	int pos = editor->insert_position();
	int found = textbuf->search_forward(pos,tfind,&pos);
	if(found) {
		textbuf->select(pos,pos+strlen(tfind));
		textbuf->remove_selection();
		textbuf->insert(pos,treplace);
		textbuf->select(pos,pos+strlen(treplace));
		editor->insert_position(pos+strlen(treplace));
		editor->show_insert_position();
	} else {
		fl_alert("No occurrences found\n");
	}
	return;
}

void find_replace_all(Fl_Text_Editor * editor, Fl_Text_Buffer * textbuf) {
	const char * tfind = replace_find->value();
	const char * treplace = replace_replace->value();
	if(!strlen(tfind)) {
		replace_dialog->show();
		return;
	}

	int pos = editor->insert_position();
	int found = textbuf->search_forward(pos,tfind,&pos);
	while(found) {
		if(found) {
			textbuf->select(pos,pos+strlen(tfind));
			textbuf->remove_selection();
			textbuf->insert(pos,treplace);
			textbuf->select(pos,pos+strlen(treplace));
			editor->insert_position(pos+strlen(treplace));
			editor->show_insert_position();
		}
		found = textbuf->search_forward(pos,tfind,&pos);
	}
	return;
}

void find_show_dialog(Fl_Widget *, void *) {
	replace_dialog->show();
	return;
}

void find_hide_dialog(Fl_Widget *, void *) {
	replace_dialog->hide();
	return;
}

void find_create_dialog(void) {
	// Create replaceace dialog
	replace_dialog = new Fl_Window(300,75,"Replace text");
	replace_find = new Fl_Input(100,0,200,globalFontsize,"Find:");
	replace_replace = new Fl_Input(100,globalFontsize,200,globalFontsize,"Replace:");
	replace_replace_all = new Fl_Button(5,50,90,globalFontsize,"Replace all");
	replace_replace_next = new Fl_Button(100,50,120,globalFontsize,"Replace next");
	replace_cancel = new Fl_Button(225,50,60,globalFontsize,"Cancel");

	replace_cancel->callback(find_hide_dialog);
	replace_replace_next->callback(editor_replace_next_callback);
	replace_replace_all->callback(editor_replace_all_callback);

	replace_dialog->add(replace_find);
	replace_dialog->add(replace_replace);
	replace_dialog->add(replace_replace_all);
	replace_dialog->add(replace_replace_next);
	replace_dialog->add(replace_cancel);
	replace_dialog->end();
	return;
}
