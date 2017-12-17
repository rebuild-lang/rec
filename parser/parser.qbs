import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "parser"
        Depends { name: "cpp" }
        Depends { name: "scanner" }
        Depends { name: "instance" }

        files: [
            "block/blockParser.cpp",
            "block/blockParser.h",
            "block/blockToken.h",
            "block/blockTokenBuilder.h",
            "block/blockTokenOutput.h",
            "expression/expressionLineView.h",
            "expression/expressionParser.cpp",
            "expression/expressionParser.h",
            "expression/expressionTree.h",
            "expression/expressionTreeBuilder.h",
            "expression/expressionTreeOutput.h",
            "filter/filterParser.cpp",
            "filter/filterParser.h",
            "filter/filterToken.h",
            "filter/filterTokenBuilder.h",
            "filter/filterTokenOutput.h",
        ]
        cpp.combineCxxSources: true

        Export {
            Depends { name: "cpp" }
            Depends { name: "scanner" }
            cpp.includePaths: [".."]
        }
    }

    Application {
        name: "parser.tests"
        consoleApplication: true
        type: ["application", "autotest"]

        Depends { name: "parser" }
        Depends { name: "googletest" }
        googletest.useMain: true

        files: [
            "block/blockParserTest.cpp",
            "expression/expressionParserTest.cpp",
            "filter/filterParserTest.cpp",
        ]
    }
}
