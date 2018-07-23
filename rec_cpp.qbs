import qbs

Project {
    minimumQbsVersion: "1.7.1"

    references: [
        "thirdparty",
        "shared",
        "scanner",
        "instance",
        "parser",
        "intrinsic",
        "api",
        "execution",
    ]

    AutotestRunner {}

    Application {
        name: "rec"
        consoleApplication: true
        Depends { name: "scanner" }
        Depends { name: "parser" }
        Depends { name: "intrinsicAdapter" }
        Depends { name: "vm" }
        Depends { name: "api" }
        files: [
            "main.cpp",
        ]
    }

    Product {
        name: "cpp17"

        Export {
            Depends { name: "cpp" }
            cpp.cxxLanguageVersion: "c++17"

            Properties {
                condition: qbs.toolchain.contains('msvc')
                cpp.cxxFlags: ["/await", "/permissive-", "/Zc:__cplusplus", "/diagnostics:caret"]
            }
            Properties {
                condition: qbs.toolchain.contains('clang')
                cpp.cxxFlags: ["-fcoroutines-ts"]
                cpp.cxxStandardLibrary: "libc++"
                cpp.staticLibraries: ["c++", "c++abi"]
            }
        }
    }

    Product {
        name: "Extra Files"
        files: [
            ".clang-format",
            ".clang-tidy",
            ".editorconfig",
            ".gitignore",
            "Makefile",
            "Readme.md",
            "Vagrantfile",
            "docs/modules.adoc",
            "docs/rebuild_API.adoc",
            "vagrant_install.sh",
            "vagrant_make.bat",
        ]
    }
}
