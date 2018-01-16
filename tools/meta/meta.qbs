import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "meta"

        files: [
            "CoEnumerator.h",
            "Flags.h",
            "LambdaPtr.h",
            "Optional.h",
            "Overloaded.h",
            "TypeList.h",
            "ValueList.h",
            "Variant.h",
            "VectorRange.h",
            "algorithm.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]
            Depends { name: "cpp17" }
        }
    }
}
