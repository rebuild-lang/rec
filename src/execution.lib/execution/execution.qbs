import qbs

Project {
    name: "execution.lib"
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "execution.lib"
        Depends { name: "cpp" }
        cpp.combineCxxSources: true
        cpp.includePaths: [".."]

        Depends { name: "instance.data" }

        files: [
            "Frame.cpp",
            "Frame.h",
            "Machine.cpp",
            "Machine.h",
            "Stack.cpp",
            "Stack.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "instance.data" }
        }
    }

    Application {
        name: "execution.tests"
        consoleApplication: true
        type: base.concat("autotest")

        Depends { name: "execution.lib" }
        Depends { name: "instance.ostream" }
        Depends { name: "googletest.lib" }
        googletest.lib.useMain: true

        files: [
            "Execution.test.cpp",
        ]
    }
}
