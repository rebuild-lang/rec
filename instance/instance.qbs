import qbs

Project {
    minimumQbsVersion: "1.7.1"

    Product {
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
            "scope.cpp",
            "scope.h",
            "scope_builder.h",
            "scope_lookup.h",
            "type.cpp",
            "type.h",
            "variable.cpp",
            "variable.h",
        ]

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
