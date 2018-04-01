import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "text"
        Depends { name: "cpp" }
        cpp.combineCxxSources: true

        Depends { name: "strings" }

        files: [
            "TextRange.cpp",
            "TextRange.h",
            "TextRangeOutput.cpp",
            "TextRangeOutput.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "strings" }
        }
    }

    StaticLibrary {
        name: "token"
        Depends { name: "cpp" }
        cpp.combineCxxSources: true

        Depends { name: "text" }

        files: [
            "NumberLiteral.h",
            "NumberLiteralOutput.cpp",
            "NumberLiteralOutput.h",
            "StringLiteral.cpp",
            "StringLiteral.h",
            "StringLiteralOutput.cpp",
            "StringLiteralOutput.h",
            "Token.cpp",
            "Token.h",
            "TokenBuilder.h",
            "TokenOutput.cpp",
            "TokenOutput.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "text" }
        }

    }

    StaticLibrary {
        name: "scanner"
        Depends { name: "cpp" }
        cpp.combineCxxSources: true

        Depends { name: "token" }

        files: [
            "CommentScanner.cpp",
            "CommentScanner.h",
            "FileInput.cpp",
            "FileInput.h",
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

            Depends { name: "token" }
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
            "CommentScannerTest.cpp",
            "IdentifierScannerTest.cpp",
            "NumberScannerTest.cpp",
            "OperatorScannerTest.cpp",
            "StringScannerTest.cpp",
            "TokenizerTest.cpp",
        ]
    }
}
