import qbs

Project {
    name: "diagnostic.data"
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "diagnostic.data"
        Depends { name: "cpp" }
        // cpp.combineCxxSources: true
        cpp.includePaths: [".."]

        Depends { name: "text.lib" }

        files: [
            "Diagnostic.cpp",
            "Diagnostic.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "text.lib" }
        }
    }
}
