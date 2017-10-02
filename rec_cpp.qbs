import qbs
import qbs.FileInfo

Project {
    minimumQbsVersion: "1.7.1"
    property string googletestPath: "C:\\C\\Lib\\googletest-1.8.0"

    StaticLibrary {
        id: googletest
        name: "googletest"

        files: [
            FileInfo.joinPaths(project.googletestPath, "googlemock/src/gmock-all.cc"),
            FileInfo.joinPaths(project.googletestPath, "googletest/src/gtest-all.cc")
        ]

        Depends { name: "cpp" }
        cpp.cxxLanguageVersion: "c++17"
        cpp.includePaths: [
            FileInfo.joinPaths(project.googletestPath, "googlemock"),
            FileInfo.joinPaths(project.googletestPath, "googlemock/include"),
            FileInfo.joinPaths(project.googletestPath, "googletest"),
            FileInfo.joinPaths(project.googletestPath, "googletest/include"),
        ]
        cpp.defines: [
            "GTEST_LANG_CXX11"
        ]

        Export {
            Depends { name: "cpp" }

            cpp.includePaths: [
                FileInfo.joinPaths(project.googletestPath, "googlemock/include"),
                FileInfo.joinPaths(project.googletestPath, "googletest/include")
            ]
            cpp.defines: [
                "GTEST_LANG_CXX11"
            ]

            property bool useMain: true
            Group {
                name: "Main"
                condition: product.googletest.useMain

                files: [
                    FileInfo.joinPaths(project.googletestPath, "googlemock/src/gmock_main.cc"),
                ]
            }
        }
    }

    references: [
        "tools",
        "scanner",
    ]

    Application {
        name: "rec"
        consoleApplication: true
        Depends { name: "scanner" }
        files: [
            "main.cpp",
        ]
    }
}
