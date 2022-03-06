import qbs

Project {
    name: "scanner.ostream"

    StaticLibrary {
        name: "scanner.ostream"
        Depends { name: "cpp" }
        cpp.combineCxxSources: true

        Depends { name: "scanner.data" }
        //Depends { name: "text.ostream" }

        files: [
            "IdentifierLiteralValue.ostream.cpp",
            "IdentifierLiteralValue.ostream.h",
            "NewLineIndentationValue.ostream.cpp",
            "NewLineIndentationValue.ostream.h",
            "NumberLiteralValue.ostream.cpp",
            "NumberLiteralValue.ostream.h",
            "StringLiteralValue.ostream.cpp",
            "StringLiteralValue.ostream.h",
            "Token.ostream.cpp",
            "Token.ostream.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "scanner.data" }
            //Depends { name: "text.ostream" }
        }
    }
}
