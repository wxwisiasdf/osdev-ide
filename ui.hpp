#ifndef UI_HPP_INCLUDED
#define UI_HPP_INCLUDED

#include <array>
#include <vector>
#include <string>
#include <regex>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Text_Buffer.H>

extern Fl_Window * gWindow;
extern Fl_Menu_Bar * gMenuBar;
Fl_Tree * ui_project_to_tree(std::array<std::vector<std::string>, 2> project);
std::regex ui_replace_dialog(Fl_Text_Buffer * buffer);
int ui_main_loop(int argc, const char ** argv);

#endif // UI_HPP_INCLUDED
