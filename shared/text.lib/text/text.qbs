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
            "File.cpp",
            "File.h",
            "FileInput.cpp",
            "FileInput.h",
            "Position.cpp",
            "Position.h",
            "Position.ostream.h",
            "TextRange.cpp",
            "TextRange.h",
            "TextRange.ostream.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "strings.lib" }
        }
    }

    //    Application {
    //        name: "text.tests"
    //        consoleApplication: true
    //        type: ["application", "autotest"]

    //        Depends { name: "text.lib" }
    //        Depends { name: "googletest.lib" }
    //        googletest.lib.useMain: true

    //        files: [
    //        ]
    //    }
}
