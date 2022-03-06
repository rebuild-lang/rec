import qbs

Project {
    StaticLibrary {
        name: "strings.lib"
        targetName: "strings"

        Depends { name: "cpp" }
        cpp.combineCxxSources: true

        Depends { name: "meta.lib" }

        files: [
            "CodePoint.cpp",
            "CodePoint.h",
            "CodePoint.ostream.h",
            "Counter.cpp",
            "Counter.h",
            "Counter.ostream.h",
            "Decoded.cpp",
            "Decoded.h",
            "Decoded.ostream.h",
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
            "join.ostream.h",
            "utf8Decode.cpp",
            "utf8Decode.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "meta.lib" }
        }
    }

    Application {
        name: "strings.tests"
        consoleApplication: true
        type: base.concat("autotest")

        Depends { name: "strings.lib" }
        Depends { name: "googletest.lib" }
        googletest.lib.useMain: true

        files: [
            "CodePoint.test.cpp",
            "Counter.test.cpp",
            "Rope.test.cpp",
            "String.test.cpp",
            "View.test.cpp",
            "join.test.cpp",
            "utf8Decode.test.cpp",
        ]
    }
}
