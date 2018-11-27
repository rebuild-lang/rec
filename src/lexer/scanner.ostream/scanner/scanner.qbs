import qbs

Project {
    minimumQbsVersion: "1.7.1"
    name: "scanner.ostream"

    StaticLibrary {
        name: "scanner.ostream"
        Depends { name: "cpp" }
        cpp.combineCxxSources: true

        Depends { name: "scanner.data" }
        //Depends { name: "text.ostream" }

        files: [
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
