import qbs

Project {
    minimumQbsVersion: "1.7.1"

    references: [
        "scanner.data/scanner",
        "scanner.ostream/scanner",
        "scanner.lib/scanner",
        "filter.data/filter",
        "filter.ostream/filter",
        "filter.lib/filter",
        "nesting.data/nesting",
        "nesting.ostream/nesting",
        "nesting.lib/nesting",
    ]
}