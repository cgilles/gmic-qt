#!/usr/bin/python

# SPDX-FileCopyrightText: 2019      by Austin Hale, <ah at unc dot edu>
# SPDX-FileCopyrightText: 2020-2024 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Export Clazy static analyzer output to HTML reports.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import sys
import argparse
import urllib.request
import re
from bs4 import BeautifulSoup
from datetime import datetime

checks_list = ["[-Wclazy-assert-with-side-effects]",
               "[-Wclazy-container-inside-loop]",
               "[-Wclazy-detaching-member]",
               "[-Wclazy-heap-allocated-small-trivial-type]",
               "[-Wclazy-ifndef-define-typo]",
               "[-Wclazy-inefficient-qlist]",
               "[-Wclazy-isempty-vs-count]",
               "[-Wclazy-jni-signatures]",
               "[-Wclazy-qhash-with-char-pointer-key]",
               "[-Wclazy-qproperty-type-mismatch]",
               "[-Wclazy-qrequiredresult-candidates]",
               "[-Wclazy-qstring-varargs]",
               "[-Wclazy-qt-keywords]",
               "[-Wclazy-qt4-qstring-from-array]",
               "[-Wclazy-qt6-qhash-signature]",
               "[-Wclazy-qt6-qlatin1char-to-u]",
               "[-Wclazy-qt6-qlatin1string-to-u]",
               "[-Wclazy-qt6-qlatin1stringchar-to-u]",
               "[-Wclazy-qt6-qdir-fixes]",
               "[-Wclazy-qvariant-template-instantiation]",
               "[-Wclazy-raw-environment-function]",
               "[-Wclazy-reserve-candidates]",
               "[-Wclazy-signal-with-return-value]",
               "[-Wclazy-thread-with-slots]",
               "[-Wclazy-tr-non-literal]",
               "[-Wclazy-unneeded-cast]",
               "[-Wclazy-use-chrono-in-qtimer]",
               "[-Wclazy-connect-by-name]",
               "[-Wclazy-connect-non-signal]",
               "[-Wclazy-connect-not-normalized]",
               "[-Wclazy-container-anti-pattern]",
               "[-Wclazy-empty-qstringliteral]",
               "[-Wclazy-fully-qualified-moc-types]",
               "[-Wclazy-lambda-in-connect]",
               "[-Wclazy-lambda-unique-connection]",
               "[-Wclazy-lowercase-qml-type-name]",
               "[-Wclazy-mutable-container-key]",
               "[-Wclazy-overloaded-signal]",
               "[-Wclazy-qcolor-from-literal]",
               "[-Wclazy-qdatetime-utc]",
               "[-Wclazy-qenums]",
               "[-Wclazy-qfileinfo-exists]",
               "[-Wclazy-qgetenv]",
               "[-Wclazy-qmap-with-pointer-key]",
               "[-Wclazy-qstring-arg]",
               "[-Wclazy-qstring-comparison-to-implicit-char]",
               "[-Wclazy-qstring-insensitive-allocation]",
               "[-Wclazy-qstring-ref]",
               "[-Wclazy-qt-macros]",
               "[-Wclazy-strict-iterators]",
               "[-Wclazy-temporary-iterator]",
               "[-Wclazy-unused-non-trivial-variable]",
               "[-Wclazy-writing-to-temporary]",
               "[-Wclazy-wrong-qevent-cast]",
               "[-Wclazy-wrong-qglobalstatic]",
               "[-Wclazy-auto-unexpected-qstringbuilder]",
               "[-Wclazy-child-event-qobject-cast]",
               "[-Wclazy-connect-3arg-lambda]",
               "[-Wclazy-const-signal-or-slot]",
               "[-Wclazy-detaching-temporary]",
               "[-Wclazy-foreach]",
               "[-Wclazy-incorrect-emit]",
               "[-Wclazy-inefficient-qlist-soft]",
               "[-Wclazy-install-event-filter]",
               "[-Wclazy-non-pod-global-static]",
               "[-Wclazy-overridden-signal]",
               "[-Wclazy-post-event]",
               "[-Wclazy-qdeleteall]",
               "[-Wclazy-qhash-namespace]",
               "[-Wclazy-qlatin1string-non-ascii]",
               "[-Wclazy-qproperty-without-notify]",
               "[-Wclazy-qstring-left]",
               "[-Wclazy-range-loop]",
               "[-Wclazy-range-loop-detach]",
               "[-Wclazy-returning-data-from-temporary]",
               "[-Wclazy-rule-of-two-soft]",
               "[-Wclazy-skipped-base-method]",
               "[-Wclazy-virtual-signal]",
               "[-Wclazy-base-class-event]",
               "[-Wclazy-copyable-polymorphic]",
               "[-Wclazy-ctor-missing-parent-argument]",
               "[-Wclazy-function-args-by-ref]",
               "[-Wclazy-function-args-by-value]",
               "[-Wclazy-global-const-char-pointer]",
               "[-Wclazy-implicit-casts]",
               "[-Wclazy-missing-qobject-macro]",
               "[-Wclazy-missing-typeinfo]",
               "[-Wclazy-old-style-connec]",
               "[-Wclazy-qstring-allocations]",
               "[-Wclazy-returning-void-expression]",
               "[-Wclazy-rule-of-three]",
               "[-Wclazy-static-pmf]",
               "[-Wclazy-virtual-call-ctor]",
]

# Each check will have its own node of information.
class checks:
    def __init__(self, dataval=None):
        self.name = ''
        self.count = 0
        self.data = ''

# Begin here.
def main():
    checks_list.sort()

    # Process command line arguments.
    args = parse_command_line_options()
    external_link = ''
    external_name = ''

    contents = args.file.readlines()

    checks_used = [0] * len(checks_list)

    # Increments each occurrence of a check.
    for line, content in enumerate(contents):
        content = content.replace('<', '&lt;')
        content = content.replace('>', '&gt;')
        for check_name in checks_list:
            if content.find(check_name) != -1:
                checks_used[checks_list.index(check_name)] += 1

    # Counts the max number of used checks in the log file.
    num_used_checks = 0
    for line, check in enumerate(checks_list):
        if checks_used[line] != 0:
            num_used_checks += 1

    names_of_used = [None] * num_used_checks
    names_of_usedL = [None] * num_used_checks

    # Creates new check classes for each used check.
    used_line = 0
    total_num_checks = 0
    for line, check in enumerate(checks_list):
        if checks_used[line] != 0:
            new_node = checks(check)
            new_node.name = check
            new_node.count = checks_used[line]
            total_num_checks += checks_used[line]
            names_of_used[used_line] = new_node

            names_of_usedL[used_line] = checks_list[line]
            used_line += 1

    # Adds details for each organized check.
    for line, content in enumerate(contents):
        # Goes through each used check.
        for initial_check in names_of_usedL:
            # Adds the lines that detail the warning message.
            if content.find(initial_check) != -1:
                content = content.replace('<', '&lt;')
                content = content.replace('>', '&gt;')
                names_of_used[names_of_usedL.index(initial_check)].data += content
                details = line + 1
                finished = False
                while not finished:
                    # Ensure there is no overflow.
                    if details >= len(contents):
                        break
                    # If the line includes a used Clang-Tidy check name,
                    # continue to find the next.
                    for end_check in names_of_usedL:
                        if contents[details].find(end_check) != -1:
                            finished = True
                            break
                    # Otherwise, add the data to the specific used check
                    # name for the organization of checks in the HTML file.
                    if not finished:
                        names_of_used[names_of_usedL.index(initial_check)].data += contents[details]
                        details += 1

    args.file.close()
    f = open("clazy.html", "w")

    # Functions for writing to the clazy.html file.
    writeHeader(f)
    writeList(f, num_used_checks, names_of_used, args,
              external_link, external_name, total_num_checks)
    sortLogs(f, contents, num_used_checks, names_of_used,
             args, external_link, external_name)
    writeScript(f, num_used_checks)

    # Close the file.
    f.close()
    sys.exit()

# Parses through the given command line options (-b, --button)
# and returns the given file's contents if read successfully.
def parse_command_line_options():
    parser = argparse.ArgumentParser()
    parser.add_argument('file', type=argparse.FileType('r'))

    try:
        args = parser.parse_args()
    except:
        parser.print_help()
        usage()
        sys.exit()

    return args

# Prints usage information for the script.
def usage():
    print("**--------------------------- Clazy Visualizer --------------------------**\n\n \
    Generates an html file as a visual for clazy checks.\n\n \
    Arguments: python clazy_visualizer.py [logfile.log]\n\n \
    \t\t-ex: python clazy_visualizer [logfile.log] \
    \n\n**------------------------------------------------------------------------**")

# Header of the clazy.html file.
def writeHeader(f):
    f.write("<!DOCTYPE html>\n")
    f.write("<html>\n")
    f.write("<head>\n")
    f.write("\t<title>Clazy Visualizer</title>\n\t<meta charset=\"UTF-8\">\n")
    f.write("\t<meta name=\"description\" content=\"Documentation tool for visualizing Clazy checks.\">\n")
    f.write("\t<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n")
    f.write("\t<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\">\n")
    f.write("\t<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>\n")
    f.write("\t<script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js\"></script>\n")
    f.write("</head>\n")

# List the used checks found in the source code.
def writeList(f, num_used_checks, names_of_used, args, external_link, external_name, total_num_checks):
    now = datetime.now()
    f.write("<body style=\"background: rgb(220, 227, 230); width: 100%; height: 100%;\">\n")
    f.write("<div id=\"container\" style=\"margin-left: 2%; margin-right: 2%;\">\n")
    f.write("\t<div id=\"header\" style=\"height: 55px; display: flex; justify-content: left; position: relative;\">\n")
    f.write("\t\t<h3 style=\"text-align: center; color: #111; font-family: 'Helvetica Neue', sans-serif; font-weight: bold; \
    letter-spacing: 0.5px; line-height: 1;\">digiKam Clazy Checks - %s</h3>\n" % (now.strftime("%m/%d/%Y %H:%M:%S")))
    f.write("\t\t<div class=\"btn-group\" role=\"group\" style=\"position: absolute; right: 0;\">\n")
    f.write("\t\t</div>\n\t</div><br>\n")
    f.write("\t<ul id=\"list\" class=\"list-group\" align=\"left\" style=\"display: block; width: 25%; height: 0; margin-bottom: 0;\">\n")

    # Iterates through each used check's details and organizes them into the given <pre> sections.
    f.write("\t\t<a id=\"log\" href=\"#\" class=\"list-group-item list-group-item-success\" style=\"color: black; font-weight: bold; letter-spacing:0.4px;\" \
    onclick=\"toggleLog()\">%d Original Log</a>\n" % (total_num_checks))

    for line in range(0, num_used_checks):
        f.write("\t\t<a id=\"check%d\" style=\"color: black\" href=\"#\" class=\"list-group-item list-group-item-action\" \
        onclick=\"toggleInfo(%d)\">%d %s</a>\n" % (line, line, names_of_used[line].count, names_of_used[line].name))

    f.write("\t</ul>\n\n")
    f.write("\t<div id=\"showLog\" style=\"display: none; width: 75%; float: right;\">\n")
    f.write("\t\t<div style=\"display: flex; justify-content: left; position: relative;\">\n")
    f.write("\t\t\t<button id=\"collapse-btn0\" type=\"button\" class=\"btn nohover\" onclick=\"collapseSidebar()\" style=\"outline: none; \
    background-color: lightgray\" title=\"Collapse sidebar\">\n")
    f.write("\t\t\t<span id=\"collapse-img0\" class=\"glyphicon glyphicon-menu-left\"></button></span>\n")
    f.write("\t\t\t<h4 style=\"margin-top: 0; color: #111; position: absolute; left: 50%; transform: translateX(-50%); margin-bottom: 10;\">Original Log</h4>\n")

    f.write("\t\t</div>\n\t\t<pre>\n")

# Sort through the used check logs for outputting the html.
def sortLogs(f, contents, num_used_checks, names_of_used, args, external_link, external_name):
    for line in contents:
        line = line.replace('<', '&lt;')
        line = line.replace('>', '&gt;')
        f.write("%s" % (line))

    f.write("\n\t\t</pre>\n\t</div>\n")

    for check_idx in range(0, num_used_checks):
        collapse_idx = check_idx+1
        f.write("\t<div id=\"show%d\"" % (check_idx))
        f.write("style=\"display: none; width: 75%; float: right\">\n")
        f.write("\t\t<div style=\"display: flex; justify-content: left; position: relative;\">\n")
        f.write("\t\t\t<button id=\"collapse-btn%d\" type=\"button\" class=\"btn nohover\" onclick=\"collapseSidebar()\" \
        style=\"outline: none; background-color: lightgray\" title=\"Collapse sidebar\">\n" % (collapse_idx))
        f.write("\t\t\t<span id=\"collapse-img%d\" class=\"glyphicon glyphicon-menu-left\"></button></span>\n" % (collapse_idx))
        f.write("\t\t\t<h4 style=\"margin-top: 0; color: #111; position: absolute; left: 50%; transform: translateX(-50%); margin-bottom: 10\">")
        f.write("<a href=\"https://github.com/KDE/clazy/blob/master/docs/checks/README-%s.md\">%s</a></h4>\n" % (names_of_used[check_idx].name[9:-1], names_of_used[check_idx].name[9:-1]))
        f.write("\t\t</div>\n\t\t<pre>\n")
        names_of_used[check_idx].data = names_of_used[check_idx].data.replace('<', '&lt;')
        names_of_used[check_idx].data = names_of_used[check_idx].data.replace('>', '&gt;')
        f.write("%s\t\t</pre>\n\t</div>\n" % (names_of_used[check_idx].data))

    f.write("</div>\n</body>\n")

# Writes Javascript and JQuery code to the html file for button and grouping functionalities.
def writeScript(f, num_used_checks):
    f.write("<script>\nvar selected_idx;\nvar checks_arr = [];\nvar highlights = 'highlights';\n")
    f.write("// Retrieves local storage data on document load for highlighted checks.\n")
    f.write("$(document).ready(function() {\n\tfor (var all_checks=0; all_checks<%d; all_checks++) {\n" % (num_used_checks))
    f.write("\t\tvar check_hl = document.getElementById(\"check\"+all_checks);\n")
    f.write("\t\tswitch (JSON.parse(localStorage.getItem(highlights))[all_checks]) {\n")
    f.write("\t\t\tcase \"warning\":\n\t\t\tcheck_hl.classList.add('list-group-item-warning');\n")
    f.write("\t\t\tchecks_arr[all_checks] = \"warning\"; break;\n\t\t\tcase \"danger\":\n")
    f.write("\t\t\tcheck_hl.classList.add('list-group-item-danger');\n\t\t\tchecks_arr[all_checks] = \"danger\"; break;\n")
    f.write("\t\t\tdefault:\n\t\t\tchecks_arr[all_checks] = \"action\";\n\t\t\tif (check_hl !== null) {\n")
    f.write("\t\t\t\tcheck_hl.classList.add('list-group-item-action');\n\t\t\t} break;\n\t\t}\n\t}\n")
    f.write("localStorage.setItem(highlights, JSON.stringify(checks_arr));\n});\n\n")

    f.write("function toggleLog() {\n\tvar log = document.getElementById(\"showLog\");\n\tclearContent();\n")
    f.write("\tif (log.style.display === \"none\") {\n\t\tlog.style.display = \"block\";\n\t} else {\n")
    f.write("\t\tlog.style.display = \"none\";\n\t}\n}\n\n")

    f.write("function toggleInfo(check_position) {\n\tselected_idx = check_position;\n\tclearContent();\n")
    f.write("\t// Displays the chosen clang-tidy category.\n\tvar category = document.getElementById(\"show\"+check_position);\n")
    f.write("\tif (category.style.display === \"none\") {\n\t\tcategory.style.display = \"block\";\n\t} else {\n")
    f.write("\t\tcategory.style.display = \"none\";\n\t}\n}\n\n")

    f.write("// Clears document when choosing another selection.\nfunction clearContent() {\n")
    f.write("\tfor (var all_checks=0; all_checks<%d; all_checks++) {\n\t\tvar clear = document.getElementById(\"show\"+all_checks);\n" % (num_used_checks))
    f.write("\t\tif (clear.style.display === \"block\") {\n\t\tclear.style.display = \"none\";\n\t\t}\n\t}\n")
    f.write("\tvar clearLog = document.getElementById(\"showLog\");\n\tif (clearLog.style.display === \"block\") {\n")
    f.write("\t\tclearLog.style.display = \"none\";\n\t}\n}\n\n")

    f.write("// Type 1 used for highlighting danger checks and 0 for warnings.\nfunction highlightChecks(type) {\n")
    f.write("\tvar check_hl = document.getElementById(\"check\"+selected_idx);\n\tif (check_hl !== null) {\n")
    f.write("\t\tif (check_hl.classList.contains('list-group-item-action')) {\n\t\t\tcheck_hl.classList.remove('list-group-item-action');\n")
    f.write("\t\t\ttype == 1 ? check_hl.classList.add('list-group-item-danger') : check_hl.classList.add('list-group-item-warning');\n")
    f.write("\t\t\ttype == 1 ? checks_arr[selected_idx] = \"danger\" : checks_arr[selected_idx] = \"warning\";\n")
    f.write("\t\t} else if (check_hl.classList.contains('list-group-item-warning')) {\n\t\t\tcheck_hl.classList.remove('list-group-item-warning');\n")
    f.write("\t\t\ttype == 1 ? check_hl.classList.add('list-group-item-danger') : check_hl.classList.add('list-group-item-action');\n")
    f.write("\t\t\ttype == 1 ? checks_arr[selected_idx] = \"danger\" : checks_arr[selected_idx] = \"action\";\n\t\t} else {\n")
    f.write("\t\t\tcheck_hl.classList.remove('list-group-item-danger');\n")
    f.write("\t\t\ttype == 1 ? check_hl.classList.add('list-group-item-action') : check_hl.classList.add('list-group-item-warning');\n")
    f.write("\t\t\ttype == 1 ? checks_arr[selected_idx] = \"action\" : checks_arr[selected_idx] = \"warning\";\n\t\t}\n\t}\n")
    f.write("\t// Sets local storage for each occurrence of a highlighted check.\n\tlocalStorage.setItem(highlights, JSON.stringify(checks_arr));\n}\n\n")

    f.write("function clearChecks(type) {\n\tfor (var all_checks=0; all_checks<%d; all_checks++) {\n" % (num_used_checks))
    f.write("\t\tvar clear = (document.getElementById(\"check\"+all_checks));\n\t\tchecks_arr[all_checks] = \"action\";\n")
    f.write("\t\tif (clear !== null) {\n")
    f.write("\t\t\tif (clear.classList.contains('list-group-item-warning')) {\n\t\t\t\tclear.classList.remove('list-group-item-warning');\n")
    f.write("\t\t\t} else if (clear.classList.contains('list-group-item-danger')) {\n\t\t\t\tclear.classList.remove('list-group-item-danger');\n\t\t\t}\n")
    f.write("\t\t\tclear.classList.add('list-group-item-action');\n\t\t}\n\t}\n\t// Restores all checks to unhighlighted state on local storage.\n")
    f.write("\tlocalStorage.removeItem(highlights);\n}\n\n")

    f.write("function collapseSidebar() {\n\tvar list = document.getElementById(\"list\"); var hasExpanded;\n")
    f.write("\tvar log_details = document.getElementById(\"showLog\");\n\tlist.style.display === \"block\" ? hasSidebar = true : hasSidebar = false;\n")
    f.write("\thasSidebar ? list.style.display = \"none\" : list.style.display = \"block\";\n")
    f.write("\tfor (var all_checks=0; all_checks<=%d; all_checks++) {\n\t\tvar collapse_img = document.getElementById(\"collapse-img\"+all_checks);\n" % (num_used_checks))
    f.write("\t\tvar collapse_btn = document.getElementById(\"collapse-btn\"+all_checks);\n\t\tvar check_details = document.getElementById(\"show\"+all_checks);\n")
    f.write("\t\tif (collapse_img !== null) {\n\t\t\thasSidebar ? collapse_img.classList.remove('glyphicon-menu-left') : collapse_img.classList.remove('glyphicon-menu-right');\n")
    f.write("\t\t\thasSidebar ? collapse_img.classList.add('glyphicon-menu-right') : collapse_img.classList.add('glyphicon-menu-left');\n")
    f.write("\t\t\thasSidebar ? collapse_btn.title = \"Expand sidebar\" : collapse_btn.title = \"Collapse sidebar\";\n\t\t}\n")
    f.write("\t\tif (check_details !== null) {hasSidebar ? check_details.style.width = \"100%\" : check_details.style.width = \"75%\";}\n\t}\n")
    f.write("\thasSidebar ? log_details.style.width = \"100%\" : log_details.style.width = \"75%\";\n}\n")

    # Begins writing style elements.
    f.write("</script>\n<style>\n\tpre {\n\t\twhite-space: pre-wrap;\n")
    f.write("\t\tword-break: keep-all;\n\t}\n\t#header {\n")
    f.write("\t\tborder-bottom: 2px solid darkgray\n\t}\n")
    f.write("</style>\n</html>")


# Calls main function.
if __name__ == "__main__":
    main()
