import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "instance"
        Depends { name: "cpp" }
        cpp.combineCxxSources: true
        cpp.includePaths: [".."]

        Depends { name: "expressionTree" }

        files: [
            "Argument.cpp",
            "Argument.h",
            "ArgumentBuilder.h",
            "ArgumentOutput.h",
            "Function.cpp",
            "Function.h",
            "FunctionBuilder.h",
            "FunctionOutput.h",
            "LocalScope.cpp",
            "LocalScope.h",
            "Module.cpp",
            "Module.h",
            "ModuleOutput.h",
            "Node.cpp",
            "Node.h",
            "Scope.cpp",
            "Scope.h",
            "ScopeBuilder.h",
            "ScopeLookup.h",
            "Type.cpp",
            "Type.h",
            "TypeOutput.h",
            "Variable.cpp",
            "Variable.h",
            "VariableOutput.h",
            "Views.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "expressionTree" }
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
