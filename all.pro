TEMPLATE = subdirs
SUBDIRS +=  src/library.pro

BUILD_EXAMPLES {
    SUBDIRS +=  examples/examples.pro 
}

DONT_BUILD_TESTS {
    message(" * Building without tests")
} else {
    SUBDIRS +=  tests/tests.pro
}