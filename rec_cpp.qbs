import qbs

Project {
    name: "Rebuild Experimental Compiler"
    minimumQbsVersion: "1.7.1"

    references: [
        "thirdparty",
        "shared",
        "src",
    ]

    AutotestRunner {}

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
        name: "[Extra Files]"
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
