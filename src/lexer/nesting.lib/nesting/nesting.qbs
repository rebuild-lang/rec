import qbs

Project {
    name: "nesting.lib"
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "nesting.lib"

        Depends { name: "nesting.data" }

        files: [
            "Parser.cpp",
            "Parser.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "nesting.data" }
        }
    }

    Application {
        name: "nesting.tests"
        consoleApplication: true
        type: ["application", "autotest"]

        Depends { name: "nesting.lib" }
        Depends { name: "nesting.ostream" }
        Depends { name: "googletest.lib" }
        googletest.lib.useMain: true

        files: [
            "blockParser.test.cpp",
        ]
    }
}
