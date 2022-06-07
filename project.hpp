#ifndef PROJECT_HPP_INCLUDED
#define PROJECT_HPP_INCLUDED

#include <string>
#include <vector>
#include <array>
#include <map>
#include <FL/Fl_Tree.H>

class Project {
public:
    std::array<std::vector<std::string>, 2> recurse_all(const std::string& root);
    void parse_make(const std::string& filename);

    // First element is rule name, second is command executed
    std::map<std::string, std::string> buildRules;
    std::array<std::vector<std::string>, 2> paths;
};

extern std::string globalCurrentDir;
extern Fl_Tree * globalProjectTree;
extern Project globalProject;
void project_open(const std::string& root);

#endif // PROJECT_HPP_INCLUDED
