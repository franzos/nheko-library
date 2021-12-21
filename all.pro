TEMPLATE = subdirs
SUBDIRS +=  src/library.pro

BUILD_EXAMPLES {
    SUBDIRS +=  examples/examples.pro 
}

BUILD_TESTS {
    SUBDIRS +=  tests/tests.pro
} else {
    message(" * Building without tests")
}