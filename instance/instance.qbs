import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "instance"
        Depends { name: "cpp" }
        Depends { name: "scanner" }

        files: [
            "argument.cpp",
            "argument.h",
            "argument_builder.h",
            "function.cpp",
            "function.h",
            "function_builder.h",
            "local_scope.cpp",
            "local_scope.h",
            "module.cpp",
            "module.h",
            "node.cpp",
            "node.h",
            "scope.cpp",
            "scope.h",
            "scope_builder.h",
            "scope_lookup.h",
            "type.cpp",
            "type.h",
            "variable.cpp",
            "variable.h",
        ]
        cpp.combineCxxSources: true

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]
        }
    }

    Application {
        condition: false
        name: "instance.tests"
        consoleApplication: true
        type: ["application", "autotest"]

        Depends { name: "instance" }
        Depends { name: "googletest" }
        googletest.useMain: true

        files: [
        ]
    }
}
