import qbs

Project {
    name: "nesting.lib"

    StaticLibrary {
        name: "nesting.lib"

        Depends { name: "nesting.data" }

        files: [
            "nestTokens.cpp",
            "nestTokens.h",
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
        type: base.concat("autotest")

        Depends { name: "nesting.lib" }
        Depends { name: "nesting.ostream" }
        Depends { name: "googletest.lib" }
        googletest.lib.useMain: true

        files: [
            "nestTokens.test.cpp",
        ]
    }
}
