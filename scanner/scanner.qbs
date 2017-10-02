import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "scanner"
        Depends { name: "cpp" }
        Depends { name: "tools" }

        files: [
            "tokenizer.cpp",
            "tokenizer.h",
        ]

        Export {
            Depends { name: "cpp" }
            Depends { name: "tools" }
            cpp.includePaths: ["."]
        }
    }

    Application {
        name: "scanner.tests"
        consoleApplication: true

        Depends { name: "scanner" }
        Depends { name: "googletest" }
        googletest.useMain: true

        files: [
            "tokenizertest.cpp"
        ]
    }
}
