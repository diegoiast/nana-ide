#include <nana/gui.hpp>
#include <nana/gui/place.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/panel.hpp>
#include <nana/gui/widgets/tabbar.hpp>
#include <nana/gui/widgets/toolbar.hpp>
#include <nana/gui/filebox.hpp>
#include <nana/threads/pool.hpp>
#include <nana/filesystem/filesystem.hpp>
#include <nana/filesystem/filesystem_ext.hpp>

#include <iostream>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <functional>

class tab_page_editor : public nana::panel<false>
{
        nana::place my_place{*this};
        nana::textbox editor{*this};

public:
        tab_page_editor(nana::window wd): nana::panel<false>(wd)
        {
                my_place.div("<editor>");
                my_place["editor"]<< editor;

                editor.set_highlight("C++ keywords", nana::colors::blue, nana::colors::white);
                editor.set_keywords("C++ keywords", false, true, { "for", "while", "break", "if", "else"});
                editor.set_highlight("C++ types", nana::colors::burly_wood, nana::colors::white);
                editor.set_keywords("C++ types", false, true, { "int", "void", "NULL"});
                editor.borderless(true);
        }
};


class project_model {
        std::vector<std::filesystem::path> files;
        std::string base_dir;
public:
        void clear() {
                base_dir.clear();
                files.clear();
        }
        void load_dir(const std::string &dir_name) {
                base_dir = dir_name;
                try {
                        for (const auto& dir : std::filesystem::recursive_directory_iterator{ dir_name }) {
                                files.push_back(dir.path());
                        }
                } catch (...) {
                }
        }

        void setup_listbox(nana::listbox &lsbox)
        {
                lsbox.clear();
                for (auto &s: files) {
                        auto relative = std::filesystem::relative(s.parent_path(), base_dir);
                        lsbox.at(0).append({ s.filename(), relative != "." ? relative : ""});
                }
        }
};

#ifdef __linux__
int exec(const char* cmd, std::function<void(const char*)> on_line_read)
{
        char line[1024];
        FILE* pipe = popen(cmd, "r");
        if (!pipe) {
                return -1;
        }
        while (fgets(line, sizeof line, pipe) != NULL) {
                on_line_read(line);
        }
        pclose(pipe);
        return 0;
}
#endif

#ifdef __WIN32__
// https://stackoverflow.com/a/46348112/78712
int exec(const char* cmd, std::function<void(const char*)> on_line_read) {
{
        // TODO
}
#endif

int main() {
        nana::threads::pool thread_pool;
        nana::form fm{};

        nana::place plc{fm};

        nana::listbox lsbox(fm);
        lsbox.append_header("filename");
        lsbox.append_header("size");

        tab_page_editor editor1(fm);
        tab_page_editor editor2(fm);
        nana::textbox lbl(fm);
        lbl.editable(false);

        nana::tabbar<std::string> tabs(fm);
        tabs.append("main.cpp", editor1);
        tabs.append("CMakeLists.txt", editor2);

        nana::toolbar tb(fm);
        tb.separate();
        tb.append("Open");
        tb.append("Compile");
        tb.append("run");

        nana::button buttonOpen{fm, "Open"};
        nana::button buttonCompile{fm, "Compile"};
        nana::button buttonRun{fm, "Run"};
        project_model project;

        buttonOpen.events().click(nana::threads::pool_push(thread_pool, [&lsbox, &project]{
                // Seems like nana 1.7.4 does not support changing the folderbox title
                nana::folderbox picker{nullptr, {}, "Choose project directory"};
                picker.title("Choose project directory");
                auto path = picker.show();
                project.clear();
                project.load_dir(path.front().string());
                project.setup_listbox(lsbox);

        }));
        buttonCompile.events().click(nana::threads::pool_push(thread_pool, [&lbl, &buttonCompile, &fm]{
                buttonCompile.enabled(false);
                buttonCompile.caption("Compiling...");
                lbl.caption("");
                exec("ninja -C build", [&lbl, &fm](const char* line){
                        lbl.append(line, false);
                });
                buttonCompile.caption("Compile");
                buttonCompile.enabled(true);
        }));
        buttonRun.events().click([](){
        });

        plc.div("vertical "
                        "<toolbar weight=32 gap=5 arrange=[100,repeated]>"
                        "<<list weight=20%>|<vert <tabs weight=32><tab_frame>|<panel weight=20%>>>"
                "");
        plc["toolbar"] << buttonOpen << buttonCompile << buttonRun;
        plc["list"] << lsbox;
        plc["tabs"] << tabs;
        plc["tab_frame"].fasten(editor1).fasten(editor2);
        plc["panel"] << lbl;
        plc.collocate();

        fm.show();
        nana::API::zoom_window(fm, true);
        nana::exec();
        return 0;
}
