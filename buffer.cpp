#include "buffer.hpp"
#include "tab.hpp"

Fl_Text_Display::Style_Table_Entry globalStyleTable[32] = {
	{FL_BLACK,		FL_COURIER,			FL_NORMAL_SIZE}, // Plain
	{FL_DARK_GREEN,	FL_COURIER,			FL_NORMAL_SIZE}, // Macros
	{FL_GRAY,		FL_COURIER_ITALIC,	FL_NORMAL_SIZE}, // Comments
	{FL_BLUE,		FL_COURIER,			FL_NORMAL_SIZE}, // Number
	{FL_BLUE,		FL_COURIER_BOLD,	FL_NORMAL_SIZE}, // Keywords
	{FL_RED,		FL_COURIER,			FL_NORMAL_SIZE}, // Strings
};

void buffer_syntax_hl(Fl_Text_Buffer * textbuf, Fl_Text_Buffer * stylebuf, int start, int end, char * new_style) {
	const char * text = textbuf->text_range(start,end+1);

	FileTab * tab;
	tab = tab_current();
	if(tab != NULL && tab->parser != NULL) {
		tab->parser(text,new_style,end-start+1);
	}
	return;
}

void buffer_restyle(Fl_Text_Buffer * textBuffer, Fl_Text_Buffer * styleBuffer) {
	int sz = textBuffer->length();
	char * style = new char[sz+1];

	// Paint everything of color black
	memset(style,'A',sz);
	style[sz] = '\0';

	// Paint depending on keywords
	buffer_syntax_hl(textBuffer,styleBuffer,0,sz+1,style);
	styleBuffer->text(style);
	return;
}
