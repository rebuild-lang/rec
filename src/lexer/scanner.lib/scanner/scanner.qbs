import qbs

Project {
    name: "scanner.lib"

    StaticLibrary {
        name: "scanner.lib"
        Depends { name: "cpp" }
        cpp.combineCxxSources: true

        Depends { name: "scanner.data" }

        files: [
            "extractComment.cpp",
            "extractComment.h",
            "extractIdentifier.cpp",
            "extractIdentifier.h",
            "extractNewLineIndentation.cpp",
            "extractNewLineIndentation.h",
            "extractNumber.cpp",
            "extractNumber.h",
            "extractOperator.cpp",
            "extractOperator.h",
            "extractString.cpp",
            "extractString.h",
            "tokenize.cpp",
            "tokenize.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "scanner.data" }
        }
    }

    Application {
        name: "scanner.tests"
        consoleApplication: true
        type: base.concat("autotest")

        Depends { name: "scanner.lib" }
        Depends { name: "scanner.ostream" }
        Depends { name: "googletest.lib" }
        googletest.lib.useMain: true

        files: [
            "extractComment.test.cpp",
            "extractIdentifier.test.cpp",
            "extractNewLineIndentation.test.cpp",
            "extractNumber.test.cpp",
            "extractOperator.test.cpp",
            "extractString.test.cpp",
            "tokenize.test.cpp",
        ]
    }
}
