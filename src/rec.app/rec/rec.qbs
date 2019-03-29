import qbs

Project {
    name: "rec.app"
    minimumQbsVersion: "1.7.1"

    Application {
        name: "rec.app"
        targetName: "rec"

        consoleApplication: true
        Depends { name: "scanner.lib" }
        Depends { name: "filter.lib" }
        Depends { name: "nesting.lib" }
        Depends { name: "parser.lib" }
        Depends { name: "intrinsic.lib" }
        Depends { name: "execution.lib" }
        Depends { name: "api.lib" }

        Depends { name: "nesting.ostream" }
        Depends { name: "scanner.ostream" }
        Depends { name: "diagnostic.ostream" }
        files: [
            "main.cpp",
        ]
    }
}
