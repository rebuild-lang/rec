import qbs

Project {
    name: "Rebuild Experimental Compiler"
    minimumQbsVersion: "1.7.1"

    references: [
        "third_party",
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
                condition: qbs.toolchain.contains('msvc') && !qbs.toolchain.contains('clang-cl')
                cpp.cxxFlags: base.concat(
                    "/permissive-", "/Zc:__cplusplus", // best C++ compatibilty
                    "/diagnostics:caret", // better error messages
                    "/await" // enable coroutine-ts
                )
            }
            Properties {
                condition: qbs.toolchain.contains('msvc') && qbs.toolchain.contains('clang-cl')
                cpp.cxxFlags: base.concat(
                    "/permissive-", "/Zc:__cplusplus", // best C++ compatibilty
                    "/diagnostics:caret", // better error messages
                    "-Xclang", "-fcoroutines-ts" // enable coroutine-ts
                )
            }
            Properties {
                condition: qbs.toolchain.contains('clang')
                cpp.cxxFlags: base.concat(
                    "--pedantic", // best C++ compatibility
                    "-Wall", "-Wextra", // enable more warnings
                    "-Wno-missing-braces", // relax bracing rules
                    "-Wno-invalid-noreturn", // we need type for type checking
                    "-Wno-gnu-zero-variadic-macro-arguments", // google test uses this
                    "-fcoroutines-ts" // enable coroutine-ts
                )
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
            ".gitattributes",
            ".gitignore",
            ".travis.yml",
            "Makefile",
            "Readme.md",
            "Vagrantfile",
            "azure-pipelines.yml",
            "docs/modules.adoc",
            "docs/rebuild_API.adoc",
            "vagrant_install.sh",
            "vagrant_make.bat",
        ]
    }
}
