#ifndef FIND_HPP_INCLUDED
#define FIND_HPP_INCLUDED

#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Text_Buffer.H>

void find_replace_next(Fl_Text_Editor * editor, Fl_Text_Buffer * textbuf);
void find_replace_all(Fl_Text_Editor * editor, Fl_Text_Buffer * textbuf);
void find_show_dialog(Fl_Widget *, void *);
void find_hide_dialog(Fl_Widget *, void *);
void find_create_dialog(void);

#endif // FIND_HPP_INCLUDED
