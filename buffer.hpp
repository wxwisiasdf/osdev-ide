#ifndef BUFFER_HPP_INCLUDED
#define BUFFER_HPP_INCLUDED

#include <FL/Fl.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Text_Display.H>

extern Fl_Text_Display::Style_Table_Entry globalStyleTable[32];

void buffer_syntax_hl(Fl_Text_Buffer * textbuf, Fl_Text_Buffer * stylebuf, int start, int end, char * new_style);
void buffer_restyle(Fl_Text_Buffer * textBuffer, Fl_Text_Buffer * styleBuffer);

#endif // BUFFER_HPP_INCLUDED
