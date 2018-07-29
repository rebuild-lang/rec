import qbs

Project {
    minimumQbsVersion: "1.7.1"
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
            "extractNumber.cpp",
            "extractNumber.h",
            "extractOperator.cpp",
            "extractOperator.h",
            "extractString.cpp",
            "extractString.h",
            "tokensFromFile.cpp",
            "tokensFromFile.h",
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
        type: ["application", "autotest"]

        Depends { name: "scanner.lib" }
        Depends { name: "scanner.ostream" }
        Depends { name: "googletest.lib" }
        googletest.lib.useMain: true

        files: [
            "extractComment.test.cpp",
            "extractIdentifier.test.cpp",
            "extractNumber.test.cpp",
            "extractOperator.test.cpp",
            "extractString.test.cpp",
            "tokensFromFile.test.cpp",
        ]
    }
}
