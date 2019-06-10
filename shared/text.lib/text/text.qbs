import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "text.lib"
        targetName: "text"

        Depends { name: "cpp" }
        cpp.combineCxxSources: true

        Depends { name: "strings.lib" }

        files: [
            "DecodedPosition.cpp",
            "DecodedPosition.h",
            "DecodedPosition.ostream.h",
            "File.cpp",
            "File.h",
            "Position.cpp",
            "Position.h",
            "Position.ostream.h",
            "Range.cpp",
            "Range.h",
            "Range.ostream.h",
            "decodePosition.cpp",
            "decodePosition.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "strings.lib" }
        }
    }

    Application {
        name: "text.tests"
        consoleApplication: true
        type: base.concat("autotest")

        Depends { name: "text.lib" }
        Depends { name: "googletest.lib" }
        googletest.lib.useMain: true

        files: [
            "Position.test.cpp",
            "decodePosition.test.cpp",
        ]
    }
}
