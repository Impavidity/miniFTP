#include "server.h"

void traverse_dir(node& n, const char * dir) {
    DIR * dp;
    struct dirent *entry;

    if ((dp = opendir(dir)) == NULL) {
        cerr << "cannot open directory " << dir << endl;
        return;
    }

    chdir(dir);
    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_type & DT_DIR) {
            if (strcmp(".", entry->d_name) == 0 ||
                strcmp("..", entry->d_name) == 0)
                continue;
            n.child.push_back(node(entry->d_name, n, true));
            traverse_dir(n.child.back(), entry->d_name);
        } else {
            n.child.push_back(node(entry->d_name, n));
        }
    }
    chdir("..");
    closedir(dp);
}

string list_dir(node* & curr) {
    string str = "";
    for (auto& item : curr->child) {
        str += item.name;
        if (item.isDir) str+="\\\n";
        else str+="\n";
    }
    return str;
}