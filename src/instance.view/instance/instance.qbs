import qbs

Project {
    name: "instance.view"

    StaticLibrary {
        name: "instance.view"

        files: [
            "Views.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]
        }
    }
}
