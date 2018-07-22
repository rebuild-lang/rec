import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "meta"

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

        Depends { name: "meta" }
        Depends { name: "googletest" }
        googletest.useMain: true

        files: [
            "Flags.test.cpp",
            "Optional.test.cpp",
            "TypeList.test.cpp",
            "Variant.test.cpp",
        ]
    }
}
