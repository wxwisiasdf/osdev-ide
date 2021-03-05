#ifndef PROJECT_HPP_INCLUDED
#define PROJECT_HPP_INCLUDED

#include <string>
#include <vector>
#include <array>
#include <FL/Fl_Tree.H>

extern std::string globalCurrentDir;
extern Fl_Tree * globalProjectTree;
std::array<std::vector<std::string>, 2> project_get_paths(std::string root);

#endif // PROJECT_HPP_INCLUDED
