import qbs

Project {
    name: "filter.lib"

    StaticLibrary {
        name: "filter.lib"
        Depends { name: "cpp" }
        cpp.includePaths: [".."]

        Depends { name: "filter.data" }

        files: [
            "filterTokens.cpp",
            "filterTokens.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "filter.data" }
        }
    }

    Application {
        name: "filter.tests"
        consoleApplication: true
        type: base.concat("autotest")

        Depends { name: "filter.lib" }
        Depends { name: "filter.ostream" }
        Depends { name: "googletest.lib" }
        googletest.lib.useMain: true

        files: [
            "filterTokens.test.cpp",
        ]
    }
}
