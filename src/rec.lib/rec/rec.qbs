import qbs

Project {
    name: "rec.lib"
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "rec.lib"

        Depends { name: "scanner.lib" }
        Depends { name: "filter.lib" }
        Depends { name: "nesting.lib" }
        Depends { name: "parser.lib" }
        Depends { name: "intrinsic.lib" }
        Depends { name: "execution.lib" }
        Depends { name: "api.lib" }

        Depends { name: "nesting.ostream" }
        Depends { name: "scanner.ostream" }
        Depends { name: "diagnostic.ostream" }
        files: [
            "Compiler.cpp",
            "Compiler.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "scanner.lib" }
            Depends { name: "filter.lib" }
            Depends { name: "nesting.lib" }
            Depends { name: "parser.lib" }
            Depends { name: "intrinsic.lib" }
            Depends { name: "execution.lib" }
            Depends { name: "api.lib" }

            Depends { name: "nesting.ostream" }
            Depends { name: "scanner.ostream" }
            Depends { name: "diagnostic.ostream" }
        }
    }
}
