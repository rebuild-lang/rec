import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "meta.lib"

        files: [
            "CoEnumerator.h",
            "Flags.h",
            "Flags.ostream.h",
            "Optional.h",
            "Optional.ostream.h",
            "Overloaded.h",
            "TypeList.h",
            "ValueList.h",
            "Variant.h",
            "Variant.ostream.h",
            "VectorRange.h",
            "algorithm.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]
            Depends { name: "cpp17" }
        }
    }

    Application {
        name: "meta.tests"
        consoleApplication: true
        type: ["application", "autotest"]

        Depends { name: "meta.lib" }
        Depends { name: "googletest.lib" }
        googletest.lib.useMain: true

        files: [
            "Flags.test.cpp",
            "Optional.test.cpp",
            "TypeList.test.cpp",
            "Variant.test.cpp",
        ]
    }
}
