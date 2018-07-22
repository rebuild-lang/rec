import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "strings"
        Depends { name: "cpp" }
        cpp.combineCxxSources: true
        cpp.includePaths: [".."]

        Depends { name: "meta" }

        files: [
            "CodePoint.cpp",
            "CodePoint.h",
            "CodePoint.ostream.h",
            "Counter.cpp",
            "Counter.h",
            "Counter.ostream.h",
            "Rope.cpp",
            "Rope.h",
            "Rope.ostream.h",
            "String.cpp",
            "String.h",
            "String.ostream.h",
            "View.cpp",
            "View.h",
            "View.ostream.h",
            "join.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "meta" }
        }
    }

    Application {
        name: "strings.tests"
        consoleApplication: true
        type: ["application", "autotest"]

        Depends { name: "strings" }
        Depends { name: "googletest" }
        googletest.useMain: true

        files: [
            "CodePoint.test.cpp",
            "Counter.test.cpp",
            "Rope.test.cpp",
            "String.test.cpp",
            "View.test.cpp",
        ]
    }
}
