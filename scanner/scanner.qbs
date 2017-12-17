import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "scanner"
        Depends { name: "cpp" }
        Depends { name: "tools" }

        files: [
            "FileInput.cpp",
            "FileInput.h",
            "NumberLiteral.h",
            "NumberLiteralOutput.cpp",
            "NumberLiteralOutput.h",
            "NumberScanner.cpp",
            "NumberScanner.h",
            "TextRange.cpp",
            "TextRange.h",
            "TextRangeOutput.cpp",
            "TextRangeOutput.h",
            "Token.cpp",
            "Token.h",
            "TokenBuilder.h",
            "TokenOutput.cpp",
            "TokenOutput.h",
            "Tokenizer.cpp",
            "Tokenizer.h",
        ]

        Export {
            Depends { name: "cpp" }
            Depends { name: "tools" }
            cpp.includePaths: [".."]
        }
        cpp.combineCxxSources: true
    }

    Application {
        name: "scanner.tests"
        consoleApplication: true
        type: ["application", "autotest"]

        Depends { name: "scanner" }
        Depends { name: "googletest" }
        googletest.useMain: true

        files: [
            "NumberScannerTest.cpp",
            "TokenizerTest.cpp",
        ]
    }
}
