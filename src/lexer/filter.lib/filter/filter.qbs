import qbs

Project {
    name: "filter.lib"
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "filter.lib"
        Depends { name: "cpp" }
        cpp.includePaths: [".."]

        Depends { name: "filter.data" }

        files: [
            "Filter.cpp",
            "Filter.h",
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
        type: ["application", "autotest"]

        Depends { name: "filter.lib" }
        Depends { name: "filter.ostream" }
        Depends { name: "googletest.lib" }
        googletest.lib.useMain: true

        files: [
            "Filter.test.cpp",
        ]
    }
}
