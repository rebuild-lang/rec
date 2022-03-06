import qbs

Project {
    name: "instance.data"

    StaticLibrary {
        name: "instance.data"
        Depends { name: "cpp" }
        //cpp.combineCxxSources: true
        cpp.includePaths: [".."]

        Depends { name: "parser.data" }
        Depends { name: "diagnostic.data" }

        files: [
            "Body.cpp",
            "Body.h",
            "Function.builder.h",
            "Function.cpp",
            "Function.h",
            "IntrinsicContext.cpp",
            "IntrinsicContext.h",
            "LocalScope.cpp",
            "LocalScope.h",
            "Module.cpp",
            "Module.h",
            "Entry.cpp",
            "Entry.h",
            "Parameter.builder.h",
            "Parameter.cpp",
            "Parameter.h",
            "Scope.builder.h",
            "Scope.cpp",
            "Scope.h",
            "ScopeLookup.h",
            "Type.builder.h",
            "Type.cpp",
            "Type.h",
            "Variable.cpp",
            "Variable.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "parser.data" }
            Depends { name: "diagnostic.data" }
        }
    }
}
