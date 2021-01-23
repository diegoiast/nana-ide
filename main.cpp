#include <iostream>
#include <nana/gui.hpp>
#include <nana/gui/place.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/panel.hpp>
#include <nana/gui/widgets/tabbar.hpp>
#include <nana/gui/widgets/toolbar.hpp>


#include <memory>
#include <vector>
#include <map>


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

int main() {
        nana::form fm;

        nana::place plc{fm};

        nana::listbox lsbox(fm);
        lsbox.append_header("filename");
        lsbox.append_header("size");
        lsbox.at(0).append({ "nana-hello.cpp", "10k" });
        lsbox.at(0).append({ "CMakeLists.txt", "879 bytes" });

        tab_page_editor editor1(fm);
        tab_page_editor editor2(fm);
        nana::label lbl(fm);

        nana::tabbar<std::string> tabs(fm);
        tabs.append("main.cpp", editor1);
        tabs.append("CMakeLists.txt", editor2);

        nana::toolbar tb(fm);
        tb.append("Open");
        tb.append("Compile");
        tb.append("run");

        plc.div("vertical "
                        "<toolbar weight=32>"
                        "<<list weight=20%>|<vert <tabs weight=32><tab_frame>|<panel weight=20%>>>"
                        "<status weight=32>"
                "");
        plc["toolbar"] << tb;
        plc["list"] << lsbox;
        plc["tabs"] << tabs;
        plc["tab_frame"].fasten(editor1).fasten(editor2);
        plc["panel"] << lbl.caption("testing");
        plc.collocate();

        fm.show();
        nana::exec();
        return 0;
}

