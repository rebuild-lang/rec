import qbs

Project {
    name: "scanner.data"
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "scanner.data"
        Depends { name: "cpp" }
        cpp.combineCxxSources: true

        Depends { name: "text.lib" }

        files: [
            "IdentifierLiteralValue.cpp",
            "IdentifierLiteralValue.h",
            "NewLineIndentationValue.cpp",
            "NewLineIndentationValue.h",
            "NumberLiteralValue.cpp",
            "NumberLiteralValue.h",
            "StringLiteralValue.cpp",
            "StringLiteralValue.h",
            "Token.cpp",
            "Token.h",
            "Token.builder.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "text.lib" }
        }

    }
}
