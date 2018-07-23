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
            "CommentScanner.cpp",
            "CommentScanner.h",
            "IdentifierScanner.cpp",
            "IdentifierScanner.h",
            "NumberScanner.cpp",
            "NumberScanner.h",
            "OperatorScanner.cpp",
            "OperatorScanner.h",
            "StringScanner.cpp",
            "StringScanner.h",
            "Tokenizer.cpp",
            "Tokenizer.h",
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
            "CommentScanner.test.cpp",
            "IdentifierScanner.test.cpp",
            "NumberScanner.test.cpp",
            "OperatorScanner.test.cpp",
            "StringScanner.test.cpp",
            "Tokenizer.test.cpp",
        ]
    }
}
