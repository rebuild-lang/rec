import qbs

Project {
    name: "diagnostic.ostream"

    StaticLibrary {
        name: "diagnostic.ostream"
        Depends { name: "cpp" }
        cpp.combineCxxSources: true
        cpp.includePaths: [".."]

        Depends { name: "diagnostic.data" }

        files: [
            "Diagnostic.ostream.cpp",
            "Diagnostic.ostream.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "diagnostic.data" }
        }
    }
}
