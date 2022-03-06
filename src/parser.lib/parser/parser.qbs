import qbs

Project {
    name: "parser.lib"

    StaticLibrary {
        name: "parser.lib"
        targetName: "parser"

        Depends { name: "instance.data" }

        files: [
            "CallErrorReporter.cpp",
            "CallErrorReporter.h",
            "CallParser.cpp",
            "CallParser.h",
            "Context.cpp",
            "Context.h",
            "LineErrorReporter.cpp",
            "LineErrorReporter.h",
            "LineView.h",
            "Parser.cpp",
            "Parser.h",
            "TupleLookup.cpp",
            "TupleLookup.h",
            "hasSideEffects.cpp",
            "hasSideEffects.h",
            "isDirectlyExecutable.cpp",
            "isDirectlyExecutable.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "instance.data" }
        }
    }

    Application {
        name: "parser.tests"
        consoleApplication: true
        type: base.concat("autotest")

        Depends { name: "parser.builder" }
        Depends { name: "parser.lib" }
        Depends { name: "parser.ostream" }
        Depends { name: "instance.ostream" }
        Depends { name: "diagnostic.ostream" }
        Depends { name: "googletest.lib" }
        googletest.lib.useMain: true

        files: [
            "CallParser.test.cpp",
            "expressionParser.test.cpp",
        ]
    }
}
