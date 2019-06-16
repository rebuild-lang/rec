import qbs

Project {
    name: "instance.data"
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "instance.data"
        Depends { name: "cpp" }
        cpp.combineCxxSources: true
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
            "Node.cpp",
            "Node.h",
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
            "TypeTree.builder.h",
            "Typed.cpp",
            "Typed.h",
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
