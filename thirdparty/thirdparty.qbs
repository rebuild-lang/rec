import qbs
import qbs.FileInfo

Project {
    minimumQbsVersion: "1.7.1"
    id: root

    property string googletestPath: "googletest"
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
        cpp.cxxStandardLibrary: {
            if (qbs.toolchain.contains('clang')) return "libc++";
        }

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
                    FileInfo.joinPaths(root.googletestPath, "googlemock/src/gmock_main.cc"),
                ]
            }

            cpp.cxxStandardLibrary: {
                if (qbs.toolchain.contains('clang')) return "libc++";
            }
            cpp.staticLibraries: {
                if (qbs.toolchain.contains('clang')) return ["pthread"];
            }
        }
    }
}
