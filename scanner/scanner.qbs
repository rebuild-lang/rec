import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "scanner"
        Depends { name: "cpp" }
        Depends { name: "tools" }

        files: [
            "file_input.cpp",
            "file_input.h",
            "number_literal.h",
            "number_scanner.cpp",
            "number_scanner.h",
            "text_range.h",
            "token.h",
            "token_builder.h",
            "tokenizer.cpp",
            "tokenizer.h",
        ]

        Export {
            Depends { name: "cpp" }
            Depends { name: "tools" }
            cpp.includePaths: [".."]
        }
    }

    Application {
        name: "scanner.tests"
        consoleApplication: true
        type: ["application", "autotest"]

        Depends { name: "scanner" }
        Depends { name: "googletest" }
        googletest.useMain: true

        files: [
            "number_scanner_test.cpp",
            "tokenizer_test.cpp",
        ]
    }
}
