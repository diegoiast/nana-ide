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
//#include <nana/filesystem/filesystem.hpp>
#include <nana/filesystem/filesystem_ext.hpp>

#define TOML11_COLORIZE_ERROR_MESSAGE
#include <toml.hpp>

#include <iostream>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <optional>

#include <string_view>

static bool endsWith(std::string_view str, std::string_view suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

static bool startsWith(std::string_view str, std::string_view prefix)
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

struct tab_page_editor : public nana::panel<false>
{
        nana::place my_place{*this};
        nana::textbox editor{*this};

        tab_page_editor(nana::window wd): nana::panel<false>(wd)
        {
                my_place.div("<editor>");
                my_place["editor"]<< editor;
                editor.borderless(true);
        }

        void load_file(const std::filesystem::path &file_path)
        {
                const std::string &file_name = file_path.filename();
                if (endsWith(file_name, "cpp") || endsWith(file_name, "c")) {
                        editor.set_highlight("C++ keywords", nana::colors::blue, nana::colors::white);
                        editor.set_keywords("C++ keywords", false, true, {"structs",  "class", "return", "void", "for", "while", "break", "if", "else"});
                        editor.set_highlight("C++ types", nana::colors::burly_wood, nana::colors::white);
                        editor.set_keywords("C++ types", false, true, { "int", "float", "char", "NULL"});
                }
#if 1
                editor.load(file_path);
#else
                std::ifstream myfile(file_path.value());
                if (myfile.is_open()) {
                        std::string line;
                        while (std::getline(myfile,line)) {
                                editor.append(line, false);
                        }
                        myfile.close();
                }
                editor.edited_reset();
//               ???
//                editor._m_saved()
#endif
        }
};

namespace std {
namespace filesystem {
        std::filesystem::path config_unix_file(std::string name)
        {
                const char *xdg_home = getenv("XDG_CONFIG_HOME");
                if (xdg_home != nullptr) {
                        path file_name = path(xdg_home);
                        file_name.replace_filename(name);
                        return file_name;
                }
                const char *unix_home = getenv("HOME");
                if (unix_home != nullptr) {
                        path file_name = path(unix_home);
                        file_name.append(name);
                        return file_name;
                }
                path file_name = std::filesystem::temp_directory_path();
                file_name.append(name);
                return file_name;
        }

        std::filesystem::path config_windows_file(std::string name)
        {
                // TODO: take ideas from https://github.com/radekp/qt/blob/master/src/corelib/io/qfsfileengine_win.cpp
                return "c:\\users\\defualt\\";
        }


        std::filesystem::path config_file(std::string name)
        {
                // TODO - windows?
                return std::filesystem::config_unix_file(name);
        }
}
}

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
                lsbox.auto_draw(false);
                for (auto &s: files) {
                        auto relative = std::filesystem::relative(s.parent_path(), base_dir);
                        lsbox.at(0).append({ s.filename(), relative != "." ? relative : ""});
                }
                lsbox.auto_draw(true);
        }

        std::optional<std::filesystem::path> get_file(size_t index)
        {
                if (index > files.size()){
                        return {};
                }
                return files.at(index);
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
int exec(const char* cmd, std::function<void(const char*)> on_line_read)
{
        // TODO
}
#endif

auto nana_ide_config_file_name = ".nanaiderc";
std::filesystem::path nana_ide_config_file;

int main() {
        nana::threads::pool thread_pool;
        nana::form fm{};
        nana::place plc{fm};
        nana::tabbar<std::string> tabs(fm);
        nana::listbox lsbox(fm);
        nana::textbox lbl(fm);
        nana::button buttonOpenFile{fm, "File"};
        nana::button buttonOpenProject{fm, "Project"};
        nana::button buttonCompile{fm, "Compile"};
        nana::button buttonRun{fm, "Run"};

        // https://www.reddit.com/r/cpp/comments/f70io2/toml_a_toml_parser_and_serializer_for_c17/
        toml::value config;
        project_model project;

        nana_ide_config_file = std::filesystem::config_file(nana_ide_config_file_name);

        lsbox.append_header("filename");
        lsbox.append_header("size");

        std::vector<std::shared_ptr<tab_page_editor>> pages;
        lsbox.events().selected([&fm, &project, &tabs, &plc, &pages](const nana::arg_listbox &msg){
                if (!msg.item.selected()) {
                        return;
                }
                std::optional<std::filesystem::path> file = project.get_file(msg.item.pos().item);
                if (file == std::nullopt) {
                        return;
                }
                if (!std::filesystem::is_regular_file(file.value())) {
                        return;
                }

                pages.push_back(std::make_shared<tab_page_editor>(fm));
                tab_page_editor &editor {*pages.back()};
                tabs.append(file.value().filename(), editor);
                editor.load_file(file.value());
                plc["tab_frame"].fasten(editor);
                editor.editor.focus();
                plc.collocate();
        });

        tabs.events().removed([&pages](const nana::arg_tabbar_removed<std::string> &arg) {
                pages.erase(pages.begin() + arg.item_pos);
        });

        lbl.editable(false);

        buttonOpenFile.events().click([&fm]{
                nana::msgbox mb(fm, "Open file");
                mb.icon(nana::msgbox::icon_information) << "unimplemented yet";
                mb.show();
        });

        buttonOpenProject.events().click(nana::threads::pool_push(thread_pool, [&lsbox, &project, &config]{
                // Seems like nana 1.7.4 does not support changing the folderbox title
                nana::folderbox picker{nullptr, {}, "Choose project directory"};
                picker.title("Choose project directory");
                auto path = picker.show();
                if (path.empty()) {
                        return;
                }
                project.clear();
                project.load_dir(path.front().string());
                project.setup_listbox(lsbox);

                try {
                        config["environment"]["loaded_dir"] = path.front().string();

                        std::ofstream config_file (nana_ide_config_file);
                        config_file << config;
                        config_file.close();
                } catch (const std::exception& e) {
                        std::cerr << e.what() << std::endl;
                }
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

        buttonRun.events().click([&fm](){
                nana::msgbox mb(fm, "Run");
                mb.icon(nana::msgbox::icon_information) << "unimplemented yet";
                mb.show();
        });

        plc.div("vert"
                "<toolbar weight=30"
                        "<left_side min=205 gap=5 arrange=[100,repeated]>"
                        "<spacer min=80>"
                        "<right_side weight=205 gap=5 arrange=[100,repeated]>"
                ">"
                "<main_window"
                        "<list weight=20%>|<vert <tabs weight=32><tab_frame>|<panel weight=20%>"
                ">"
        );
        plc["left_side"] << buttonOpenFile;
        plc["left_side"] << buttonOpenProject;
        plc["right_side"] << buttonRun;
        plc["right_side"] << buttonCompile;

        plc["list"] << lsbox;
        plc["tabs"] << tabs;
        plc["panel"] << lbl;
        plc.collocate();

        try {
                config = toml::parse<toml::preserve_comments>(nana_ide_config_file);
                const std::string loaded_dir = toml::find<std::string>(config, "environment", "loaded_dir");
                if (!loaded_dir.empty()) {
                        project.clear();
                        project.load_dir(loaded_dir);
                        project.setup_listbox(lsbox);
                } else {
                        // TODO: hacks to see something on screen
                        tab_page_editor editor1(fm);
                        tab_page_editor editor2(fm);
                        tabs.append("main.cpp", editor1);
                        tabs.append("CMakeLists.txt", editor2);
                        plc["tab_frame"].fasten(editor1).fasten(editor2);
                }
        } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
        }

        fm.show();

        // TODO - we should ave geometry of window on config
        nana::API::zoom_window(fm, true);
        nana::exec();
        return 0;
}
