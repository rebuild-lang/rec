import qbs

Project {
    name: "intrinsic.data"

    StaticLibrary {
        name: "intrinsic.data"
        Depends { name: "cpp" }
        cpp.includePaths: [".."]

        Depends { name: "parser.data" }

        files: [
            "Function.cpp",
            "Function.h",
            "Module.h",
            "Parameter.h",
            "Type.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "parser.data" }
        }
    }
}
