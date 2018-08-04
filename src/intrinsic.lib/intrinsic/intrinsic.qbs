import qbs

Project {
    name: "intrinsic.lib"
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "intrinsic.lib"
        Depends { name: "cpp" }
        cpp.includePaths: [".."]

        Depends { name: "instance.data" }
        Depends { name: "intrinsic.data" }

        files: [
            "Adapter.cpp",
            "Adapter.h",
            "ResolveType.cpp",
            "ResolveType.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "instance.data" }
            Depends { name: "intrinsic.data" }
        }
    }

    Application {
        name: "intrinsic.tests"
        consoleApplication: true
        type: ["application", "autotest"]

        Depends { name: "intrinsic.lib" }
        Depends { name: "intrinsic.ostream" }
        Depends { name: "googletest.lib" }
        googletest.lib.useMain: true

        files: [
            "Adapter.test.cpp",
        ]
    }
}
