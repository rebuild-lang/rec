import qbs

Project {
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
