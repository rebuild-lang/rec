import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "vm"
        Depends { name: "cpp" }
        Depends { name: "tools" }
        Depends { name: "instance" }
        cpp.combineCxxSources: true
        cpp.includePaths: [".."]

        files: [
            "Machine.cpp",
            "Machine.h",
            "Scope.cpp",
            "Scope.h",
            "Stack.cpp",
            "Stack.h",
        ]

        Export {
            Depends { name: "cpp" }
            Depends { name: "tools" }
            Depends { name: "instance" }
            cpp.includePaths: [".."]
        }
    }

//    StaticLibrary {
//        name: "expression_converter"
//        Depends { name: "cpp" }
//        cpp.combineCxxSources: true
//        cpp.includePaths: [".."]

//        files: [
//        ]

//        Export {
//            Depends { name: "cpp" }
//            cpp.includePaths: [".."]
//        }
//    }

    Application {
        name: "execution.tests"
        consoleApplication: true
        type: ["application", "autotest"]

        Depends { name: "vm" }
        Depends { name: "googletest" }
        googletest.useMain: true

        files: [
            "ExecutionTests.cpp"
        ]
    }
}
