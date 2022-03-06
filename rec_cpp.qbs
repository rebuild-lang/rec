import qbs

Project {
    name: "Rebuild Experimental Compiler"

    // Customizations:
    // - enforce debug symbols for debuggig
    //   modules.cpp.debugInformation:true

    references: [
        "third_party",
        "shared",
        "src",
    ]

    AutotestRunner {}

    Product {
        name: "cpp20"

        Export {
            Depends { name: "cpp" }
            cpp.cxxLanguageVersion: "c++20"
            cpp.treatWarningsAsErrors: true
            cpp.enableRtti: false

            Properties {
                condition: qbs.toolchain.contains('msvc')
                cpp.cxxFlags: base.concat(
                    "/permissive-", "/Zc:__cplusplus", "/Zc:inline", // best C++ compatibilty
                    "/diagnostics:caret", // better error messages
                    "/W4", // enable all warnings
                    "/external:anglebrackets", "/external:W0" // ignore warnings from external headers
                )
            }
            Properties {
                condition: qbs.toolchain.contains('clang') || qbs.toolchain.contains('gcc')
                cpp.cxxFlags: base.concat(
                    "--pedantic", // best C++ compatibility
                    "-Wall", "-Wextra", // enable more warnings
                    "-Wno-missing-braces", // relax bracing rules
                    "-Wno-invalid-noreturn", // we need type for type checking
                    "-Wno-gnu-zero-variadic-macro-arguments" // google test uses this
                )
            }
            Properties {
                condition: qbs.toolchain.contains('clang')
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
            ".github/workflows/test_runner.yml",
            ".gitignore",
            "Makefile",
            "Readme.md",
            "docker_clang_check.sh",
            "docker_gcc_check.sh",
            "docker_qtc_clang.sh",
            "docker_qtc_gcc.sh",
            "docs/modules.adoc",
            "docs/rebuild_API.adoc",
        ]
    }
}
