import qbs

Project {
    name: "rec.lib"
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "rec.lib"

        Depends { name: "scanner.lib" }
        Depends { name: "filter.lib" }
        Depends { name: "nesting.lib" }
        Depends { name: "parser.lib" }
        Depends { name: "intrinsic.lib" }
        Depends { name: "execution.lib" }
        Depends { name: "api.lib" }

        Depends { name: "nesting.ostream" }
        Depends { name: "scanner.ostream" }
        Depends { name: "instance.ostream" }
        Depends { name: "diagnostic.ostream" }
        files: [
            "Compiler.cpp",
            "Compiler.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "scanner.lib" }
            Depends { name: "filter.lib" }
            Depends { name: "nesting.lib" }
            Depends { name: "parser.lib" }
            Depends { name: "intrinsic.lib" }
            Depends { name: "execution.lib" }
            Depends { name: "api.lib" }

            Depends { name: "nesting.ostream" }
            Depends { name: "scanner.ostream" }
            Depends { name: "instance.ostream" }
            Depends { name: "diagnostic.ostream" }
        }
    }

    Application {
        name: "rec.tests"
        consoleApplication: true
        type: base.concat("autotest")

        Depends { name: "rec.lib" }
        Depends { name: "googletest.lib" }
        googletest.lib.useMain: true

        files: [
            "LexerErrors.test.cpp",
        ]
    }
}
